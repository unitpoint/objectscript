#if defined(_WIN32)
# include "stdafx.h"
# include "windows.h"
#endif

#include "../../source/objectscript.h"
#include "../../source/os-binder.h"

#ifndef OS_CURL_DISABLED
#include "../../source/ext-curl/os-curl.h"
#endif

#ifndef OS_SQLITE3_DISABLED
#include "../../source/ext-sqlite3/os-sqlite3.h"
#endif

#ifndef OS_REGEXP_DISABLED
#include "../../source/ext-regexp/os-regexp.h"
#endif

#ifndef OS_OPENGL_DISABLED
#include "../../source/ext-opengl/os-opengl.h"
#include <glut.h>
#endif

#ifndef OS_LIBNOISE_DISABLED
#include "../../source/ext-libnoise/os-libnoise.h"
#endif

using namespace ObjectScript;

static double inv_frequency = 0.0;
static double start_time = 0.0;

#if defined(_WIN32)
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
#endif

double getTimeSec()
{
#if defined(_WIN32)
	LARGE_INTEGER largeInteger;
	QueryPerformanceCounter(&largeInteger);
	double count = double(largeInteger.QuadPart);
	return inv_frequency * (count - start_time);
#else
	return start_time;
#endif
}

#if defined(_WIN32)
int _tmain(int argc, _TCHAR* _argv[])
{
	char ** argv = new char*[argc];
	{ for(int i = 0; i < argc; i++){
		int len = 0; for(; _argv[i][len]; len++);
		argv[i] = new char[len+1];
		for(int j = 0; j <= len; j++){
			argv[i][j] = (char)_argv[i][j];
		}
	} }
	struct FreeArgv {
		int argc;
		char ** argv;
		~FreeArgv(){
			for(int i = 0; i < argc; i++){
				delete [] argv[i];
			}
			delete [] argv;
		}
	} __free_argvc__ = {argc, argv};
#else
int main(int argc, char *argv[])
{
#endif
	if(argc < 2){
		printf("ObjctScript " OS_VERSION " Copyright (C) 2012-2013 Evgeniy Golovin (evgeniy.golovin@unitpoint.ru)\n");
		printf("Latest version and source code: https://github.com/unitpoint/objectscript\n");
		printf("\n");
		printf("Usage: %s script.os [args]\n", argv[0]);
		exit(1);
	}

	// create ObjectScript instance
#if 0
	class MyOS: public OS
	{
	public:
		MyOS(){}
		
		void initPreScript()
		{
			setSetting(OS_SETTING_CREATE_TEXT_EVAL_OPCODES, true);
			OS::initPreScript();
		}
	};
	OS * os = OS::create(new MyOS());
#else
	OS * os = OS::create();
#endif
	
#ifndef OS_CURL_DISABLED
	initCurlLibrary(os);
#endif

#ifndef OS_SQLITE3_DISABLED
	initSqlite3Library(os);
#endif

#ifndef OS_REGEXP_DISABLED
	initRegexpLibrary(os);
#endif

#ifndef OS_OPENGL_DISABLED
	glutInit(&argc, argv);
	initOpenglLibrary(os);
#endif

#ifndef OS_LIBNOISE_DISABLED
	initLibNoiseLibrary(os);
#endif

	// save allocated memory at start point
	int start_mem_usage = os->getAllocatedBytes();
	// set needed settings
	os->setSetting(OS_SETTING_CREATE_DEBUG_INFO, true);
	os->setSetting(OS_SETTING_CREATE_TEXT_OPCODES, true);
	os->setSetting(OS_SETTING_CREATE_COMPILED_FILE, true);
	// create program arguments
	os->newObject();
	for(int i = 0; i < argc; i++){
		os->pushStackValue(-1);
		os->pushNumber(i-1);
		os->pushString(argv[i]);
		os->setProperty();
	}
	// we can use the program arguments as global arg variable inside of our script
	os->setGlobal("arg");
	// set global getTimeSec function so we can check time inside of our script
	os->setGlobal(def("getTimeSec", getTimeSec));
	// run main stript
	// os->require("c:\\Sources\\OS\\proj.win32\\profile_benchmark\\scripts\\n-body.os"); // getString(os, argv[1]));
	// os->require("c:\\Sources\\OS\\unit-tests-os\\operators.os"); // getString(os, argv[1]));
	// os->require(getString(os, argv[1]));
#if 1
	os->require(argv[1], true, 0, OS_SOURCECODE_AUTO);
#else
	os->compileFile(getString(os, argv[1]));
	os->pushNull();
	os->call();
#endif
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
