#include "os-binder.h"

using namespace ObjectScript;

// =====================================================================
// =====================================================================
// =====================================================================

static OS_FunctionDataChain * function_data_first = NULL;

OS_FunctionDataChain::OS_FunctionDataChain()
{ 
	next = function_data_first;
	function_data_first = this;
}
OS_FunctionDataChain::~OS_FunctionDataChain()
{
}

void OS_finalizeAllBinds()
{
	while(function_data_first){
		OS_FunctionDataChain * cur = function_data_first;
		function_data_first = cur->next;
		delete cur;
	}
}

struct OS_FunctionDataFinalizer
{
	~OS_FunctionDataFinalizer(){ OS_finalizeAllBinds(); }
} __functionDataFinalizer__;

// =====================================================================
// =====================================================================
// =====================================================================
