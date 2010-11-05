#include "rtl.h"
#include <string.h>

char * RtlDuplicateString(const char *string)
{
	size_t size = strlen(string) + 1;
	void *result = RtlAllocateHeap(size);

	if (result != NULL)
		memcpy(result, string, size);
	
	return (char *)result;
}
