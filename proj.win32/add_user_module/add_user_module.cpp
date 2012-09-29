#include "stdafx.h"
#include "../../source/objectscript.h"

using namespace ObjectScript;

#include <ctype.h>

static int ctype_digit(OS * os, int params, int, int, void*)
{
	OS::String str = os->toString(-params);
	int len = str.getLen();
	if(len == 0){
		os->pushBool(false);
		return 1;
	}
	for(int i = 0; i < len; i++){
		if(!isdigit(str[i])){
			os->pushBool(false);
			return 1;
		}
	}
	os->pushBool(true);
	return 1;
}

static int ctype_space(OS * os, int params, int, int, void*)
{
	OS::String str = os->toString(-params); \
	int len = str.getLen();
	if(len == 0){
		os->pushBool(false);
		return 1;
	}
	for(int i = 0; i < len; i++){
		if(!isspace(str[i])){
			os->pushBool(false);
			return 1;
		}
	}
	os->pushBool(true);
	return 1;
}

void initCtypeModule(OS * os)
{
	OS::FuncDef funcs[] = {
		{"isdigit", ctype_digit},
		{"isspace", ctype_space},
		{}
	};
	os->getModule(OS_TEXT("ctype"));
	os->setFuncs(funcs);
	os->pop();
}

int _tmain(int argc, _TCHAR* argv[])
{
	// craete ObjectScript instance
	OS * os = OS::create();

	// init ctype module
	initCtypeModule(os);

	// run program
	os->require("../../examples-os/add_user_module.os");

	// release the ObjectScript instance
	os->release();
	return 0;
}

