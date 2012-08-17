// OS.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "..\source\objectscript.h"

using namespace ObjectScript;

static OS::String readFile(OS * os, const char * filename)
{
	FILE * f = fopen(filename, "r");
	if(!f){
		return OS::String(os);
	}
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	rewind(f);

	OS::String data(os, OS_TEXT('\0'), size);
	fread((void*)data.toChar(), size, 1, f);
	fclose(f);

	return data;
}

struct __test_os__
{
	__test_os__()
	{
		OS * os = OS::create();
		const char * filename = "test-data/castlemonster.js";
		// const char * filename = "test.os";
		OS::String test_prog = readFile(os, filename);
		
		// os->eval("abc, x, y = 5, var1*6 + var2*3  x = y-7;");
		// os->eval("y(8, i)(i*8+j[7, 9])[7, k]");
		// os->eval("abc, x = 5, 37676  hjhj = y-7;");
		// os->eval("abc* (5+ 37676 * 9.4 % 17.3) * [7 + k*j, j, k]  - (hjhj + y-7);");
		// os->eval("abc, x = 5, 37676  hjhj = y-7;");
		// os->eval("1 + {x:9*u, \"y\":9*u, 12:9*u, 12.1:9*u, [9*u]: 9*u, 9*u} * 8");
		os->eval(test_prog.getDataSize() ? test_prog.toChar() :
			"var a; var b = 0; var i, j = 1, 2;\n"
			"a[b][c] = a[b][c];\n"
			"a,b = a.b.c;\n"
			"a.b.c.d[a, b.v].a = a.b.c.d.e(a, b) * 5;\n"
			"a.b.c.d[a, b.v] = a.b.c.d.e(a, b) * 5;\n"
			"a.b[a, b] = (5 * b, a.b[a, b]) * 5;\n"
			"a.b.c = a.b.c * 5;\n"
			"a.b = a.b * 5;\n"
			"a, b = 4, 5, 6;\n"
			"a, b = 5;\n"
			"a, b = ((\n"
			"function(x, y, z)\n"
			"{\n"
			"	return x*a + y*z,\n"
			"			y ** 2;\n"
			"})(a(),2,3));\n"
			"b(); 5, t, 6, a(), 6;");
		os->pushInt(123);
		os->newObject();
		os->pushString("qwerty");
		os->getProperty();
		os->pop();
		os->newObject();
		os->pushString("qwerty");
		os->pushFloat(1.23);
		os->setProperty();
		os->pushFloat(56.0);
		os->pushFloat(1.23);
		os->setProperty();
		os->pushInt(-56);
		os->pushString("abc");
		os->setProperty();
		os->pushStackValue();
		os->pushFloat(-56.0);
		os->getProperty();
		os->pop();
		os->pushStackValue();
		os->pushString("-56");
		os->getProperty();
		OS::String str = os->toString();
		os->pop();

		os->pushInt(-56);
		int * val = (int*)os->pushUserData(sizeof(int));
		*val = 1;		
		os->setProperty();
		os->popAll();

		int mem_usage = os->getAllocatedBytes();
		os->newObject();
		for(int i = 0; i < 10000; i++){
			os->pushString(OS::String(os, i)+"-index");
			// os->pushInt(i);
			os->pushFloat(1.0 / i);
			os->setProperty();
		}
		int mem_usage2 = os->getAllocatedBytes();

		// os->pushInt(100);
		os->pushString(OS::String(os, 100)+"-index");
		os->getProperty();
		OS_FLOAT check_val = os->toNumber();

		os->popAll();
		int mem_usage3 = os->getAllocatedBytes();

		os->release();
	}
} __test_os__;

int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}

