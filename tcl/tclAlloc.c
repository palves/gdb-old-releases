#include "tclInt.h"

char *
Tcl_Malloc(size)
	unsigned int size;
{
	return malloc(size);
}

char *
Tcl_Realloc(ptr, size)
	char *ptr;
	unsigned int size;
{
	return realloc(ptr, size);
}

void
Tcl_Free(ptr)
	char *ptr;
{
	free(ptr);
}
