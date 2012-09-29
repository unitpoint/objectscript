#include "stdafx.h"
#include "../../source/objectscript.h"

using namespace ObjectScript;

int _tmain(int argc, _TCHAR* argv[])
{
	// craete ObjectScript instance
	OS * os = OS::create();

	// run program
	os->require("../../examples-os/run_os_prog.os");

	// release the ObjectScript instance
	os->release();
	return 0;
}

