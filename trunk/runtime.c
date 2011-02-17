#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>

void runtime_abort(char *what)
{
	fprintf(stderr, "%s\n", what);
	exit(1);
}

void runtime_ignore(char *what)
{
	fprintf(stderr, "%s\n", what);
}
