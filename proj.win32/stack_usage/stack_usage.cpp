#include "stdafx.h"
#include "../../source/objectscript.h"

using namespace ObjectScript;

int _tmain(int argc, _TCHAR* argv[])
{
	// craete ObjectScript instance
	OS * os = OS::create();

	/*
		Part 1: let's simulate following OS code:

		print("10 * (3+2) = ", 10 * (3+2))
	*/

	// prepare function call
	os->getGlobal("print");	// #1 - stack values, it's print function from standart library
	os->pushNull();			// #2 - null, it's function this, each call of function must have this
	// push the first argument
	os->pushString("10 * (3+2) = ");	// #3 - we have 3 stack values here
	// prepare second argument
	os->pushNumber(10);		// #4
	os->pushNumber(3);		// #5
	os->pushNumber(2);		// #6
	os->runOp(OP_ADD);		// #5 - 3+2
	os->runOp(OP_MUL);		// #4 - 10 * (3+2)
	// we have 4 stack values here:
	// #1 - function
	// #2 - function this, it's null here
	// #3 - string "10 * (3+2) = "
	// #4 - number, it's result of 10 * (3+2), it's 50
	// we are ready to call function with arguments
	os->call(2); // call function with 2 arguments
	// #0 - called function remove all used stack values
	// please see console to view output of this example

	/*
		Part 2: let's simulate following OS code:

		bar = {firsname="James", lastname="Bond"}
		bar.profession = "actor"
		print bar
	*/

	os->newObject();		// #1 - new object
	
	os->pushStackValue(-1);		// #2 - the same object, -1 - is relative pointer to the top stack value
	os->pushString("firsname");	// #3 - property key
	os->pushString("James");	// #4 - property value
	os->setProperty();			// #1 - setProperty uses 3 stack values and pop them
	
	os->pushStackValue(-1);		// #2
	os->pushString("lastname");	// #3 - property key
	os->pushString("Bond");		// #4 - property value
	os->setProperty();			// #1

	os->setGlobal("bar");		// #0 - assign object value to global bar variable, pop value

	// let's do bar.profession = "actor"
	os->getGlobal("bar");			// #1 - our global a variable
	os->pushString("profession");	// #2 - property key
	os->pushString("Bond");		// #4 - property value
	os->setProperty();			// #0

	// let's do print bar
	os->getGlobal("print");		// #1
	os->pushNull();				// #2
	os->getGlobal("bar");		// #3
	os->call(1);				// #0

	/*
		Part 3: let's simulate following OS code:

		print(concat(5, " big differences"))
	*/
	os->getGlobal("print");		// #1 - print function
	os->pushNull();				// #2 - this for print
	os->getGlobal("concat");	// #3 - concat function
	os->pushNull();				// #4 - this for concat
	os->pushNumber(5);			// #5
	os->pushString(" big differences"); // #6
	// call concat function with 2 arguments and 1 requested result value
	// the call pops 2 arguments + 1 this + 1 function and pushes 1 result value
	os->call(2, 1);				// #3 - result is already at the top of stack
	// call print function with 1 arguments and 0 requested result values
	os->call(1);				// #0

	// release the ObjectScript instance
	os->release();
	return 0;
}

