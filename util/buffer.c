#include <stdlib.h>
#include <httpd_util.h>

CSTATUS BufferInit(
	OUT BUFFER *Buffer,
	size_t InitialAllocSize)
{
	Buffer->Data = (char *)malloc(InitialAllocSize);
	if (!Buffer->Data)
		return C_BAD_ALLOC;

	Buffer->AllocSize = InitialAllocSize;
	return C_SUCCESS;
}

void BufferUninit(
	BUFFER *Buffer)
{
	free(Buffer->Data);
}

CSTATUS BufferExpand(
	BUFFER *Buffer,
	size_t MinAllocSize)
{
	char *NewData;
	size_t NewAllocSize;

	if (MinAllocSize > Buffer->AllocSize) {

		NewAllocSize = Buffer->AllocSize;
		do {
			NewAllocSize *= 2;
		} while (MinAllocSize > NewAllocSize);

		NewData = (char *)realloc(Buffer->Data, NewAllocSize);
		if (!NewData)
			return C_BAD_ALLOC;

		Buffer->Data = NewData;
		Buffer->AllocSize = NewAllocSize;
	}

	return C_SUCCESS;
}
