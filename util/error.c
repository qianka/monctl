#include "stdio.h"
#include "stdlib.h"


void
error(const char *msg)
{
	fprintf(stderr, "Error: %s\n", msg);
	exit(-1);
}
