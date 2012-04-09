#include <assert.h>
#include <string.h>
#include <httpd_util.h>

CSTATUS BufferedReaderInit(
	OUT BUFFERED_READER *Reader,
	size_t InitialAllocSize,
	READ_FUNCTION *ReadFunction,
	void *Context)
{
	CSTATUS status;

	status = BufferInit(&Reader->Buffer, InitialAllocSize);
	if (!SUCCESS(status))
		return status;

	Reader->DataSize = 0;
	Reader->NextOffset = 0;
	Reader->ReadFunction = ReadFunction;
	Reader->Context = Context;
	return C_SUCCESS;
}

void BufferedReaderUninit(
	BUFFERED_READER *Reader)
{
	BufferUninit(&Reader->Buffer);
}

CSTATUS BufferedReaderReadLine(
	BUFFERED_READER *Reader,
	size_t ReadSize,
	size_t MaxAllocSize,
	int TrimRight,
	OUT size_t *Offset)
{
	CSTATUS status;
	size_t offset = Reader->NextOffset;
	size_t actualSize;
	char *first, *cur;

	while (1) {

		// read fresh data if necessary
		assert(offset <= Reader->DataSize);
		if (offset == Reader->DataSize) {
			if (offset >= MaxAllocSize)
				return C_LIMIT_EXCEEDED;

			status = BufferExpand(&Reader->Buffer, offset + ReadSize);
			if (!SUCCESS(status))
				return status;

			status = Reader->ReadFunction(Reader->Context, &Reader->Buffer.Data[offset], ReadSize, &actualSize);
			if (!SUCCESS(status))
				return status;

			if (actualSize == 0)
				return C_END_OF_FILE;

			Reader->DataSize += actualSize;
		}

		assert(offset < Reader->DataSize);
		if (Reader->Buffer.Data[offset] == '\n') {
			*Offset = Reader->NextOffset;
			first = &Reader->Buffer.Data[Reader->NextOffset];
			Reader->NextOffset = offset + 1;
			cur = &Reader->Buffer.Data[offset];

			// remove line separators
			*cur-- = '\0';
			if (cur >= first && *cur == '\r')
				*cur-- = '\0';

			// remove trailing LWS
			if (TrimRight)
				while (cur >= first && (*cur == ' ' || *cur == '\t'))
					*cur-- = '\0';

			return C_SUCCESS;
		}

		++offset;
	}
}

CSTATUS BufferedReaderRead(
	BUFFERED_READER *Reader,
	char *Buffer,
	size_t Size,
	OUT size_t *ActualSize)
{
	size_t offset;
	size_t actualSize;

	offset = Reader->NextOffset;

	// if there is remaining data in the buffer,
	// a read operation should NOT be performed
	if (offset < Reader->DataSize) {
		actualSize = Reader->DataSize - offset;
		if (actualSize > Size)
			actualSize = Size;

		memcpy(Buffer, &Reader->Buffer.Data[offset], actualSize);
		*ActualSize = actualSize;
		Reader->NextOffset = offset + actualSize;
		return C_SUCCESS;
	}

	// read fresh data
	return Reader->ReadFunction(Reader->Context, Buffer, Size, ActualSize);
}

void BufferedReaderFlush(
	BUFFERED_READER *Reader)
{
	size_t offset = Reader->NextOffset;
	size_t size = Reader->DataSize - offset;

	memmove(&Reader->Buffer.Data[0], &Reader->Buffer.Data[offset], size);
	Reader->NextOffset = 0;
	Reader->DataSize = size;
}
