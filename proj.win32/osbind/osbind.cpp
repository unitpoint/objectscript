#include "stdafx.h"
#include "../../source/objectscript.h"
#include "../../source/os-binder.h"
#include <ctype.h>
#include <direct.h>

using namespace ObjectScript;

std::string getcwdString()
{
	const int PATH_MAX = 1024;
	char buf[PATH_MAX];
	getcwd(buf, PATH_MAX);
	return buf;
}

bool my_isdigit(const OS::String& str)
{
	int len = str.getLen();
	for(int i = 0; i < len; i++){
		if(!isdigit(str[i])){
			return false;
		}
	}
	return len > 0;
}

std::string my_hash(const char * str)
{
	int i, len = strlen(str), hash = 5381;
	for(i = 0; i < len; i++){
		hash = ((hash << 5) + hash) + str[i];
	}
	hash &= 0x7fffffff;
	char buf[16];
	for(i = 0; hash > 0; hash >>= 4){
		buf[i++] = "0123456789abcdef"[hash & 0xf];
	}
	buf[i] = 0;
	return buf;
}

void my_print_num(int i)
{
	printf("my_print_num: %d\n", i);
}

void my_print_void(void)
{
	printf("my_print_void\n");
}

long double my_fabs(long double a)
{
	return a >= 0 ? a : -a;
}

// ========================================

class TestClass
{
public:
	int i;
	float j;

	TestClass(int _i, float _j)
	{ 
		i = _i; j = _j;
	}
	~TestClass()
	{
		// you could set breakpoint here to be sure that this object is deleted automaticaly
	}

	int getI() const { return i; }
	void setI(int _i){ i = _i; }
	
	float getJ() const { return j; }
	void setJ(float _j){ j = _j; }

	TestClass * clone()
	{
		return new TestClass(*this);
	}

	TestClass * add(TestClass * b)
	{
		return new TestClass(i + b->i, j + b->j);
	}

	double doSomething(int a, float b, double c, TestClass * pb)
	{
		return i + j + a + b + c + pb->i + pb->j;
	}

	void print()
	{
		printf("test class: %d, %f\n", i, j);
	}
};

namespace ObjectScript { OS_DECL_USER_CLASS(TestClass); }

TestClass * __constructTestClass(int i, float j)
{
	return new TestClass(i, j);
}

void registerTestClass(OS * os)
{
	OS::FuncDef funcs[] = {
		def("__construct", __constructTestClass),
		def("__get@i", &TestClass::getI),
		def("__set@i", &TestClass::setI),
		def("__get@j", &TestClass::getJ),
		def("__set@j", &TestClass::setJ),
		def("clone", &TestClass::clone),
		def("__add", &TestClass::add),
		def("doSomething", &TestClass::doSomething),
		def("print", &TestClass::print),
		{}
	};
	registerUserClass<TestClass>(os, funcs);
}

// ========================================

class NewTestClass: public TestClass
{
public:

	NewTestClass(): TestClass(10, 20){}

	double doSomething(int a, float b, double c, TestClass * pb)
	{
		return i - j - a - b - c - pb->i - pb->j;
	}
};

namespace ObjectScript { OS_DECL_USER_CLASS(NewTestClass); }

NewTestClass * __constructNewTestClass()
{
	return new NewTestClass();
}

void registerNewTestClass(OS * os)
{
	OS::FuncDef funcs[] = {
		def("__construct", __constructNewTestClass),
		def("doSomething", &NewTestClass::doSomething),
		{}
	};
	registerUserClass<NewTestClass, TestClass>(os, funcs);
}

// ========================================

void initMyModule(OS * os)
{
	OS::FuncDef funcs[] = {
		def("isdigit", my_isdigit),
		def("hash", my_hash),
		def("print_num", my_print_num),
		def("print_void", my_print_void),
		def("abs", my_fabs),
		def("fabs", (double(__cdecl*)(double))fabs),
		{}
	};
	os->getModule("my");
	os->setFuncs(funcs);
	os->pop();
}

struct TestStruct
{
	float a, b;

	TestStruct(){ a = b = 0; }
	TestStruct(float _a, float _b){ a = _a; b = _b; }
};

void printTestStruct(const TestStruct& p)
{
	printf("TestStruct: %f %f\n", p.a, p.b);
}

TestStruct changeTestStruct(const TestStruct& p)
{
	return TestStruct(p.a*10, p.b*100);
}

namespace ObjectScript {

OS_DECL_USER_CLASS(TestStruct);

template <>
struct CtypeValue<TestStruct>
{
	typedef TestStruct type;

	static bool isValid(const TestStruct&){ return true; }

	static TestStruct def(ObjectScript::OS * os){ return TestStruct(0, 0); }
	static TestStruct getArg(ObjectScript::OS * os, int offs)
	{
		if(os->isObject(offs)){
			os->getProperty(offs, "a"); // required
			float a = os->popFloat();
		
			os->getProperty(offs, "b"); // required
			float b = os->popFloat();

			return TestStruct(a, b);
		}
		os->setException("TestStruct expected");
		return TestStruct(0, 0);
	}

	static void push(ObjectScript::OS * os, const TestStruct& p)
	{
		os->newObject();
	
		os->pushStackValue();
		os->pushNumber(p.a);
		os->setProperty("a");
				
		os->pushStackValue();
		os->pushNumber(p.b);
		os->setProperty("b");
	}
};

} // namespace ObjectScript

int _tmain(int argc, _TCHAR* argv[])
{
	// craete ObjectScript instance
	OS * os = OS::create();

	registerTestClass(os);
	registerNewTestClass(os);
	initMyModule(os);

	os->setGlobal(def("getcwd", getcwdString));
	os->setGlobal(def("printTestStruct", printTestStruct));
	os->setGlobal(def("changeTestStruct", changeTestStruct));

	// run program
	os->require("../../examples-os/bind.os");

	// release the ObjectScript instance
	os->release();
	return 0;
}

