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

int RtlConcatString(char **string, const char *source)
{
	size_t targetLength = strlen(*string);
	size_t sourceLength = strlen(source);
	
	if (RtlReallocateHeap((void *)string, sourceLength + targetLength + 1)) {
		return 1;
	}
	
	memcpy(&(*string)[targetLength], source, sourceLength + 1);
	return 0;
}
