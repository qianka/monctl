#include <stdlib.h>

void
subprocess(const char *cmd, int *status,
	   size_t stdout_size, char *stdout,
	   size_t stderr_size, char *stderr);
