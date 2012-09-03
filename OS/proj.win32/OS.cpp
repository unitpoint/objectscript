// OS.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "..\source\objectscript.h"
#include <windows.h>

using namespace ObjectScript;

/*
static OS::String readFile(OS * os, const char * filename)
{
	FILE * f = fopen(filename, "rb");
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
		// const char * filename = "test-data/castlemonster.js";
		// const char * filename = "test-data/test.os";
		// const char * filename = "test-data/test2.os";
		const char * filename = "test-data/test_fannkuch.os";
		// OS::String test_prog = readFile(os, filename);

		os->run(filename);
		
		os->newObject(); // 1 - t1
		os->newObject(); // 2 - t2

		os->pushStackValue(-1); // 3 - t2
		os->pushString("qwerty"); // 4
		os->pushStackValue(-4); // 5 - t1
		os->setProperty(false); // 2

		os->pushStackValue(-2); // 3 - t1
		os->pushString("qwerty"); // 4
		os->pushStackValue(-3); // 5 - t2
		os->setProperty(false); // 2

		os->removeAll();

		/?
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
		?/

		{
			int mem_usage = os->getAllocatedBytes();
			os->gcFull();
			int after_mem_usage = os->getAllocatedBytes();
			int i = mem_usage;
		}
		
		os->pushNumber(123);
		os->newObject();
		os->pushString("qwerty");
		os->getProperty();
		os->pop();
		
		os->newObject();
		
		os->pushStackValue(-1);
		os->pushString("qwerty");
		os->pushNumber(1.23);
		os->setProperty();

		os->pushStackValue(-1);
		os->pushNumber(56.0);
		os->pushNumber(1.23);
		os->setProperty();

		os->pushStackValue(-1);
		os->pushNumber(-56);
		os->pushString("abc");
		os->setProperty();

		os->pushStackValue();
		os->pushNumber(-56.0);
		os->getProperty();

		os->pop();

		os->pushStackValue();
		os->pushString("-56");
		os->getProperty();

		// OS::Value value = os->getValue();
		// os->pushValue(value);
		OS::String str = os->toString();
		bool boolean = os->toBool();
		os->pop();

		os->pushNumber(-56);
		int * val = (int*)os->pushUserData(0, sizeof(int));
		*val = 1;		
		os->setProperty(true);
		os->removeAll();

		int mem_usage = os->getAllocatedBytes();
		os->newObject();
		for(int i = 0; i < 100000; i++){
			os->pushStackValue(-1);
			os->pushString(OS::String(os, i)+"-index");
			// os->pushInt(i);
			os->pushNumber(1.0 / i);
			os->setProperty();
		}
		int mem_usage2 = os->getAllocatedBytes();

		// os->pushInt(100);
		os->pushString(OS::String(os, 100)+"-index");
		os->getProperty();
		OS_FLOAT check_val = os->toNumber();

		os->removeAll();
		{
			int mem_usage = os->getAllocatedBytes();
			os->gcFull();
			int after_mem_usage = os->getAllocatedBytes();
			int i = mem_usage;
		}
		os->release();
	}
} __test_os__;
*/

OS::String getString(OS * os, const _TCHAR * str)
{
	char buf[1024*10];
	int i = 0;
	for(; str[i]; i++){
		buf[i] = (char)str[i];
	}
	buf[i] = 0;
	return OS::String(os, buf);
}

static double inv_frequency = 0.0;
static double start_time = 0.0;

struct __init_time__
{
	__init_time__()
	{
		LARGE_INTEGER largeInteger;

		if(inv_frequency == 0.0f){
			QueryPerformanceFrequency(&largeInteger);
			inv_frequency = double(largeInteger.QuadPart);
			if(inv_frequency > 0.0f){
				inv_frequency = 1.0f / inv_frequency;
			}
		}

		QueryPerformanceCounter(&largeInteger);
		start_time = double(largeInteger.QuadPart);
	}
} __init_time__;

double getTimeSec()
{
	LARGE_INTEGER largeInteger;
	QueryPerformanceCounter(&largeInteger);
	double count = double(largeInteger.QuadPart);
	return inv_frequency * (count - start_time);
}

static int OS_getTimeSec(OS * os, int params, int upvalues, int, void*)
{
	os->pushNumber(getTimeSec());
	return 1;
}

int _tmain(int argc, _TCHAR* argv[])
{
	OS * os = OS::create();

	// debug
	// os->setMemBreakpoint(8417);

	if(argc < 2){
		printf("ObjctScript " OS_VERSION " Copyright (C) 2012 Evgeniy Golovin (evgeniy.golovin@unitpoint.ru)\n");
		printf("Latest version and source code: https://github.com/unitpoint/objectscript\n");
		printf("\n");
		printf("Usage: %s script [args]\n", getString(os, argv[0]).toChar());

		// os->release();
		exit(1);
	}

	double start_time = getTimeSec();

	os->setSetting(OS_SETTING_CREATE_DEBUG_INFO, true);
	os->setSetting(OS_SETTING_CREATE_DEBUG_OPCODES, true);
	os->setSetting(OS_SETTING_RECOMPILE_SOURCECODE, true);

	os->newArray();
	for(int i = 0; i < argc; i++){
		os->pushStackValue(-1);
		os->pushNumber(i-1);
		os->pushString(getString(os, argv[i]));
		os->setProperty();
		// os->addProperty();
	}
	os->setGlobal("arg");

	os->pushCFunction(OS_getTimeSec);
	os->setGlobal("getTimeSec");

	int start_mem_usage = os->getAllocatedBytes();
	os->run(getString(os, argv[1])); // argc >= 2 && argv[1] ? getString(os, argv[1]) : "test-data/test3.os");
	// exit(1);
	// os->eval("print arg");
	{
		int mem_allocated = os->getAllocatedBytes()/1024;
		int mem_cached = os->getCachedBytes()/1024;
		os->gcFull();
		int after_mem_allocated = os->getAllocatedBytes()/1024;
		int after_mem_cached = os->getCachedBytes()/1024;
		printf("\n\nMem allocated: %d %d (Kb), cached: %d %d (Kb), used: %d %d (Kb)\n", 
			mem_allocated, after_mem_allocated, 
			mem_cached, after_mem_cached,
			mem_allocated - mem_cached, after_mem_allocated - after_mem_cached);
	}

	os->release();
	return 0;
}

