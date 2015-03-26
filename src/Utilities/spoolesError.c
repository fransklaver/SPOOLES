#include <stdlib.h>

#include "Utilities/spoolesError.h"

static void defaultExit(void)
{
	spoolesFatal();
}

static ExitFunction spoolesFail = defaultExit;

ExitFunction registerExitFunction(ExitFunction func)
{
	ExitFunction old;

	if (!func)
		func = defaultExit;

	old = spoolesFail;
	spoolesFail = func;
	return old;
}

void spoolesFatal(void)
{
	spoolesFail();
}
