#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <httpd_util.h>

CSTATUS BufferedWriterInit(
	OUT BUFFERED_WRITER *Writer,
	size_t InitialAllocSize,
	size_t BlockSize,
	WRITE_FUNCTION *WriteFunction,
	void *Context)
{
	CSTATUS status;

	status = BufferInit(&Writer->Buffer, InitialAllocSize);
	if (!SUCCESS(status))
		return status;

	Writer->DataSize = 0;
	Writer->BlockSize = BlockSize;
	Writer->WriteFunction = WriteFunction;
	Writer->Context = Context;
	return C_SUCCESS;
}

void BufferedWriterUninit(
	BUFFERED_WRITER *Writer)
{
	BufferedWriterFlush(Writer); // may fail
	BufferUninit(&Writer->Buffer);
}

CSTATUS BufferedWriterFlush(
	BUFFERED_WRITER *Writer)
{
	CSTATUS status;
	char *data = Writer->Buffer.Data;
	size_t remaining = Writer->DataSize;
	size_t actualSize;

	while (1) {
		status = Writer->WriteFunction(Writer->Context, data, remaining, &actualSize);
		if (!SUCCESS(status))
			break;
		data += actualSize;
		if ((remaining -= actualSize) == 0)
			break;
	}

	memmove(Writer->Buffer.Data, data, remaining);
	Writer->DataSize = remaining;
	return status;
}

CSTATUS BufferedWriterWrite(
	BUFFERED_WRITER *Writer,
	const char *Buffer,
	size_t Size,
	OUT size_t *ActualSize)
{
	CSTATUS status;

	if (Size < Writer->BlockSize) {
		status = BufferExpand(&Writer->Buffer, Writer->DataSize + Size);
		if (!SUCCESS(status))
			return status;

		memcpy(&Writer->Buffer.Data[Writer->DataSize], Buffer, Size);
		if ((Writer->DataSize += Size) >= Writer->BlockSize)
			BufferedWriterFlush(Writer); // may fail

		*ActualSize = Size;
		return C_SUCCESS;
	} else {
		status = BufferedWriterFlush(Writer);
		if (!SUCCESS(status))
			return status;
		return Writer->WriteFunction(Writer->Context, Buffer, Size, ActualSize);
	}
}

CSTATUS BufferedWriterVPrintf(
	BUFFERED_WRITER *Writer,
	const char *Format,
	va_list ArgList)
{
	CSTATUS status;
	int count;

	count = _vscprintf(Format, ArgList);
	status = BufferExpand(&Writer->Buffer, Writer->DataSize + count + 1);
	if (!SUCCESS(status))
		return status;

	count = vsprintf(&Writer->Buffer.Data[Writer->DataSize], Format, ArgList);
	if (count < 0)
		return ErrnoToCStatus(errno);

	if ((Writer->DataSize += count) >= Writer->BlockSize)
		BufferedWriterFlush(Writer); // may fail

	return C_SUCCESS;
}

CSTATUS BufferedWriterPrintf(
	BUFFERED_WRITER *Writer,
	const char *Format, ...)
{
	CSTATUS status;
	va_list args;

	va_start(args, Format);
	status = BufferedWriterVPrintf(Writer, Format, args);
	va_end(args);

	return status;
}
