// OS.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "..\source\objectscript.h"

using namespace ObjectScript;

struct __test_os__
{
	__test_os__()
	{
		OS * os = OS::create();
		// os->eval("abc, x, y = 5, var1*6 + var2*3  x = y-7;");
		// os->eval("y(8, i)(i*8+j[7, 9])[7, k]");
		// os->eval("abc, x = 5, 37676  hjhj = y-7;");
		// os->eval("abc* (5+ 37676 * 9.4 % 17.3) * [7 + k*j, j, k]  - (hjhj + y-7);");
		// os->eval("abc, x = 5, 37676  hjhj = y-7;");
		// os->eval("1 + {x:9*u, \"y\":9*u, 12:9*u, 12.1:9*u, [9*u]: 9*u, 9*u} * 8");
		os->eval(
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
			"	return x + y*z,\n"
			"			y ** 2;\n"
			"})(a(),2,3));\n"
			"b(); 5, t, 6, a(), 6;");
		os->pushInt(123);
		os->pushObject();
		os->pushString("qwerty");
		os->objectGet();
		os->pop();
		os->pushObject();
		os->pushString("qwerty");
		os->pushFloat(1.23);
		os->objectSet();
		os->pushFloat(56.0);
		os->pushFloat(1.23);
		os->objectSet();
		os->pushInt(-56);
		os->pushString("abc");
		os->objectSet();
		os->pushStackValue();
		os->pushFloat(-56.0);
		os->objectGet();
		os->pop();
		os->pushStackValue();
		os->pushString("-56");
		os->objectGet();
		OS::String str = os->toString();
		os->pop();

		os->pushInt(-56);
		int * val = (int*)os->pushUserdata(sizeof(int));
		*val = 1;		
		os->objectSet();
		os->popAll();

		int mem_usage = os->getAllocatedBytes();
		os->pushObject();
		for(int i = 0; i < 10000; i++){
			os->pushString(OS::String(os, i)+"-index");
			// os->pushInt(i);
			os->pushFloat(1.0 / i);
			os->objectSet();
		}
		int mem_usage2 = os->getAllocatedBytes();

		// os->pushInt(100);
		os->pushString(OS::String(os, 100)+"-index");
		os->objectGet();
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

