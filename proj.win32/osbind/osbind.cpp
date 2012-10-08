#include "stdafx.h"
#include "../../source/objectscript.h"
#include "../../source/os-binder.h"
#include <ctype.h>

using namespace ObjectScript;

static bool my_isdigit(const OS::String& str)
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

	TestClass * add(TestClass * a, TestClass * b)
	{
		return new TestClass(a->i + b->i, a->j + b->j);
	}

	double doSomething(int a, float b, double c)
	{
		return i + j + a + b + c;
	}

	void print()
	{
		printf("test class: %d, %f\n", i, j);
	}
};

OS_DECL_USER_CLASS(TestClass);

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
		def("__clone", &TestClass::clone),
		def("__add", &TestClass::add),
		def("doSomething", &TestClass::doSomething),
		def("print", &TestClass::print),
		{}
	};
	registerUserClass<TestClass>(os, funcs);
}

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

int _tmain(int argc, _TCHAR* argv[])
{
	// craete ObjectScript instance
	OS * os = OS::create();

	registerTestClass(os);
	initMyModule(os);

	// run program
	os->require("../../examples-os/bind.os");

	// release the ObjectScript instance
	os->release();
	return 0;
}

