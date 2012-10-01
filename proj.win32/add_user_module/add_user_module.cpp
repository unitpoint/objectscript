#include "stdafx.h"
#include "../../source/objectscript.h"

using namespace ObjectScript;

#include <ctype.h>

static int my_isdigit(OS * os, int params, int, int, void*)
{
	OS::String str = os->toString(-params);
	int len = str.getLen();
	for(int i = 0; i < len; i++){
		if(!isdigit(str[i])){
			os->pushBool(false);
			return 1;
		}
	}
	os->pushBool(len > 0);
	return 1;
}

static int my_hash(OS * os, int params, int, int, void*)
{
	OS::String str = os->toString(-params);
	int i, len = str.getLen(), hash = 5381;
	for(i = 0; i < len; i++){
		hash = ((hash << 5) + hash) + str[i];
	}
	hash &= 0x7fffffff;
	char buf[16];
	for(i = 0; hash > 0; hash >>= 4){
		buf[i++] = "0123456789abcdef"[hash & 0xf];
	}
	buf[i] = 0;
	os->pushString(buf);
	return 1;
}

void initMyModule(OS * os)
{
	OS::FuncDef funcs[] = {
		{"isdigit", my_isdigit},
		{"hash", my_hash},
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

	// init ctype module
	initMyModule(os);

	// run program
	os->require("../../examples-os/add_user_module.os");

	// release the ObjectScript instance
	os->release();
	return 0;
}

