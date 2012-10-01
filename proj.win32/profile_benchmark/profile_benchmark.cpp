#include "stdafx.h"
#include "windows.h"

#define OPCODES_BUF_SIZE 255 // it's not real number of opcodes but enough to store all opcodes

OpcodeProfile opcodes_usage[OPCODES_BUF_SIZE];
double start_opcode_time = 0;

struct __init_opcodes_usage__
{
	__init_opcodes_usage__()
	{
		for(int i = 0; i < OPCODES_BUF_SIZE; i++){
			opcodes_usage[i].opcode = i;
		}
	}
} __init_opcodes_usage__;

int compareOpcodeProfile(const void * a, const void * b)
{
	OpcodeProfile * p[] = {(OpcodeProfile*)a, (OpcodeProfile*)b};
	return p[0]->time < p[1]->time ? 1 : p[0]->time > p[1]->time ? -1 : 0;
}

void sortOpcodeProfile()
{
	qsort(opcodes_usage, OPCODES_BUF_SIZE, sizeof(opcodes_usage[0]), compareOpcodeProfile);
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

#include "../../source/objectscript.h"

using namespace ObjectScript;

int gc_step_count = 0;
double gc_step_start_time = 0, gc_time = 0;

class ProfileOS: public OS
{
public:

	ProfileOS(){}
	
	void onEnterGC()
	{
		gc_step_start_time = getTimeSec();
	}

	void onExitGC()
	{
		gc_step_count++;
		gc_time += getTimeSec() - gc_step_start_time;
	}

	static const char * opcodeName(int opcode)
	{
#define OPCODE_NAME(name) case Core::Program::name: return #name
		switch(opcode){
		OPCODE_NAME(OP_PUSH_NUMBER);
		OPCODE_NAME(OP_PUSH_STRING);
		OPCODE_NAME(OP_PUSH_NULL);
		OPCODE_NAME(OP_PUSH_TRUE);
		OPCODE_NAME(OP_PUSH_FALSE);
		OPCODE_NAME(OP_PUSH_FUNCTION);
		OPCODE_NAME(OP_PUSH_NEW_ARRAY);
		OPCODE_NAME(OP_PUSH_NEW_OBJECT);
		OPCODE_NAME(OP_OBJECT_SET_BY_AUTO_INDEX);
		OPCODE_NAME(OP_OBJECT_SET_BY_EXP);
		OPCODE_NAME(OP_OBJECT_SET_BY_INDEX);
		OPCODE_NAME(OP_OBJECT_SET_BY_NAME);
		OPCODE_NAME(OP_PUSH_ENV_VAR);
		OPCODE_NAME(OP_PUSH_ENV_VAR_AUTO_CREATE);
		OPCODE_NAME(OP_SET_ENV_VAR);
		OPCODE_NAME(OP_PUSH_THIS);
		OPCODE_NAME(OP_PUSH_ARGUMENTS);
		OPCODE_NAME(OP_PUSH_REST_ARGUMENTS);
		OPCODE_NAME(OP_PUSH_LOCAL_VAR);
		OPCODE_NAME(OP_PUSH_LOCAL_VAR_BY_AUTO_INDEX);
		OPCODE_NAME(OP_PUSH_LOCAL_VAR_AUTO_CREATE);
		OPCODE_NAME(OP_SET_LOCAL_VAR);
		OPCODE_NAME(OP_PUSH_UP_LOCAL_VAR);
		OPCODE_NAME(OP_PUSH_UP_LOCAL_VAR_AUTO_CREATE);
		OPCODE_NAME(OP_SET_UP_LOCAL_VAR);
		OPCODE_NAME(OP_CALL);
		OPCODE_NAME(OP_TAIL_CALL);
		OPCODE_NAME(OP_CALL_METHOD);
		OPCODE_NAME(OP_TAIL_CALL_METHOD);
		OPCODE_NAME(OP_GET_PROPERTY);
		OPCODE_NAME(OP_GET_PROPERTY_AUTO_CREATE);
		OPCODE_NAME(OP_SET_PROPERTY);
		OPCODE_NAME(OP_SET_DIM);
		OPCODE_NAME(OP_IF_NOT_JUMP);
		OPCODE_NAME(OP_JUMP);
		OPCODE_NAME(OP_DEBUGGER);
		OPCODE_NAME(OP_EXTENDS);
		OPCODE_NAME(OP_DELETE_PROP);
		OPCODE_NAME(OP_RETURN);
		OPCODE_NAME(OP_RETURN_AUTO);
		OPCODE_NAME(OP_POP);
		OPCODE_NAME(OP_LOGIC_AND);
		OPCODE_NAME(OP_LOGIC_OR);
		OPCODE_NAME(OP_COMPARE);
		OPCODE_NAME(OP_LOGIC_PTR_EQ);
		OPCODE_NAME(OP_LOGIC_PTR_NE);
		OPCODE_NAME(OP_LOGIC_EQ);
		OPCODE_NAME(OP_LOGIC_NE);
		OPCODE_NAME(OP_LOGIC_GE);
		OPCODE_NAME(OP_LOGIC_LE);
		OPCODE_NAME(OP_LOGIC_GREATER);
		OPCODE_NAME(OP_LOGIC_LESS);
		OPCODE_NAME(OP_BIT_AND);
		OPCODE_NAME(OP_BIT_OR);
		OPCODE_NAME(OP_BIT_XOR);
		OPCODE_NAME(OP_ADD);
		OPCODE_NAME(OP_SUB);
		OPCODE_NAME(OP_MUL);
		OPCODE_NAME(OP_DIV);
		OPCODE_NAME(OP_MOD);
		OPCODE_NAME(OP_LSHIFT);
		OPCODE_NAME(OP_RSHIFT);
		OPCODE_NAME(OP_POW);
		OPCODE_NAME(OP_CONCAT);
		OPCODE_NAME(OP_BIT_NOT);
		OPCODE_NAME(OP_PLUS);
		OPCODE_NAME(OP_NEG);
		OPCODE_NAME(OP_LENGTH);
		OPCODE_NAME(OP_LOGIC_BOOL);
		OPCODE_NAME(OP_LOGIC_NOT);
		OPCODE_NAME(OP_IN);
		OPCODE_NAME(OP_ISPROTOTYPEOF);
		OPCODE_NAME(OP_IS);
		OPCODE_NAME(OP_SUPER);
		OPCODE_NAME(OP_SUPER_CALL);
		OPCODE_NAME(OP_TYPE_OF);
		OPCODE_NAME(OP_VALUE_OF);
		OPCODE_NAME(OP_NUMBER_OF);
		OPCODE_NAME(OP_STRING_OF);
		OPCODE_NAME(OP_ARRAY_OF);
		OPCODE_NAME(OP_OBJECT_OF);
		OPCODE_NAME(OP_USERDATA_OF);
		OPCODE_NAME(OP_FUNCTION_OF);
		OPCODE_NAME(OP_CLONE);
		OPCODE_NAME(OP_NOP);
		}
		return "UNKNOWN OPCODE !!!";
	}
};

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

static int OS_getTimeSec(OS * os, int, int, int, void*)
{
	os->pushNumber(getTimeSec());
	return 1;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// craete ObjectScript instance
	OS * os = OS::create(new ProfileOS());

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
	os->pushCFunction(OS_getTimeSec);
	os->setGlobal("getTimeSec");

	// run program
	double start_os_time = getTimeSec();
	os->require(argc >= 2 ? getString(os, argv[1]).toChar() : "../os/test_fannkuch.os"); // "../../examples-os/profile_benchmark.os");
	double full_time = getTimeSec() - start_os_time;

#if defined _DEBUG || defined OS_DEBUG
	printf("=============================\n");
	printf(" WARNING: debug mode enabled\n");
	printf("=============================\n");
#endif

	printf("OS script full time: %f secs\n", full_time);
	printf("            gc time: %f secs (%.2f%%)\n", gc_time, gc_time*100/full_time);

	sortOpcodeProfile();
	int i;
	for(i = 0; i < 10; i++){
		printf("\n=== %s, executed count: %d\n", ProfileOS::opcodeName(opcodes_usage[i].opcode), opcodes_usage[i].count);
		printf("sum time: %f secs (%.2f%%), avg time of 100000 times: %f\n", 
			opcodes_usage[i].time, opcodes_usage[i].time*100/full_time, 
			opcodes_usage[i].time * 100000 / opcodes_usage[i].count);
	}
	int sum_count = 0;
	double sum_time = 0;
	for(i = 0; i < OPCODES_BUF_SIZE; i++){
		sum_count += opcodes_usage[i].count;
		sum_time += opcodes_usage[i].time;
	}
	printf("\n=== ALL OPCODES, executed count: %d\n", sum_count);
	printf("sum time: %f secs (%.2f%%), avg time of 100000 times: %f\n", 
		sum_time, sum_time*100/full_time, 
		sum_time * 100000 / sum_count);

	// release the ObjectScript instance
	os->release();
	return 0;
}
