#include "stdafx.h"
#include "windows.h"
#include "../../source/objectscript.h"
#include "../../source/os-binder.h"

using namespace ObjectScript;

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

OS::String getString(OS * os, const _TCHAR * str)
{
	OS_CHAR buf[1024*10];
	int i = 0;
	for(; str[i]; i++){
		buf[i] = (OS_CHAR)str[i];
	}
	buf[i] = 0;
	return OS::String(os, buf);
}

double getTimeSec()
{
	LARGE_INTEGER largeInteger;
	QueryPerformanceCounter(&largeInteger);
	double count = double(largeInteger.QuadPart);
	return inv_frequency * (count - start_time);
}

int _tmain(int argc, _TCHAR* argv[])
{
	if(argc < 2){
		printf("ObjctScript " OS_VERSION " Copyright (C) 2012 Evgeniy Golovin (evgeniy.golovin@unitpoint.ru)\n");
		printf("Latest version and source code: https://github.com/unitpoint/objectscript\n");
		printf("\n");
		printf("Usage: %s script [args]\n", argv[0]);
		exit(1);
	}

	// craete ObjectScript instance
	OS * os = OS::create();
	// save allocated memory at start point
	int start_mem_usage = os->getAllocatedBytes();
	// set needed settings
	os->setSetting(OS_SETTING_CREATE_DEBUG_INFO, true);
	os->setSetting(OS_SETTING_CREATE_DEBUG_OPCODES, true);
	os->setSetting(OS_SETTING_CREATE_COMPILED_FILE, true);
	// create program arguments
	os->newObject();
	for(int i = 0; i < argc; i++){
		os->pushStackValue(-1);
		os->pushNumber(i-1);
		os->pushString(getString(os, argv[i]));
		os->setProperty();
	}
	// we can use the program arguments as global arg variable inside of our script
	os->setGlobal("arg");
	// set global getTimeSec function so we can check time inside of our script
	os->setGlobal(def("getTimeSec", getTimeSec));
	// run main stript
	os->require(getString(os, argv[1]));
	// os->compileFile(getString(os, argv[1]));
	{
		int mem_allocated = os->getAllocatedBytes()/1024;
		int mem_cached = os->getCachedBytes()/1024;
		// run gc full step
		os->gcFull();
		int after_mem_allocated = os->getAllocatedBytes()/1024;
		int after_mem_cached = os->getCachedBytes()/1024;
		// output some debug memory usage info
		printf("\n\n[before GC] memory used: %d Kb, cached: %d Kb, allocated: %d Kb\n[after  GC] memory used: %d Kb, cached: %d Kb, allocated: %d Kb\n", 
			mem_allocated - mem_cached, 
			mem_cached, 
			mem_allocated, 
			after_mem_allocated - after_mem_cached,
			after_mem_cached,
			after_mem_allocated
			);
#ifdef OS_DEBUG
		printf("\nNotice: debug build uses much more memory than release build\n");
#endif
	}
	// release the ObjectScript instance
	os->release();
	return 0;
}
