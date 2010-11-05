#include "rtl.h"
#include <stdlib.h>

void * RtlAllocateHeap(size_t size)
{
	return malloc(size);
}

int RtlReallocateHeap(void *pPointer, size_t newSize)
{
	void *result = realloc(*(void **)pPointer, newSize);
	
	if (result == NULL) {
		return 1;
	}
	
	*(void **)pPointer = result;
	return 0;
}

void RtlFreeHeap(void *pointer)
{
	free(pointer);
}
