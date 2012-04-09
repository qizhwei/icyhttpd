#ifndef HTTPD_UTIL_H
#define HTTPD_UTIL_H

#include <stddef.h>
#include <stdint.h>
#include <cstatus.h>
#include <misc.h>

#ifdef __cplusplus
extern "C" {
#endif

// Buffer
typedef struct _BUFFER {
	char *Data;
	size_t AllocSize;
} BUFFER;

extern CSTATUS BufferInit(
	OUT BUFFER *Buffer,
	size_t InitialAllocSize);

extern void BufferUninit(
	BUFFER *Buffer);

extern CSTATUS BufferExpand(
	BUFFER *Buffer,
	size_t MinAllocSize);

// BufferedReader
typedef CSTATUS READ_FUNCTION(
	void *Context,
	char *Buffer,
	size_t Size,
	OUT size_t *ActualSize);
	
typedef struct _BUFFERED_READER {
	BUFFER Buffer;
	size_t DataSize;
	size_t NextOffset;
	READ_FUNCTION *ReadFunction;
	void *Context;
} BUFFERED_READER;

extern CSTATUS BufferedReaderInit(
	OUT BUFFERED_READER *Reader,
	size_t InitialAllocSize,
	READ_FUNCTION *ReadFunction,
	void *Context);

extern void BufferedReaderUninit(
	BUFFERED_READER *Reader);

extern CSTATUS BufferedReaderReadLine(
	BUFFERED_READER *Reader,
	size_t ReadSize,
	size_t MaxAllocSize,
	int TrimRight,
	OUT size_t *Offset);

extern CSTATUS BufferedReaderRead(
	BUFFERED_READER *Reader,
	char *Buffer,
	size_t Size,
	OUT size_t *ActualSize);

extern void BufferedReaderFlush(
	BUFFERED_READER *Reader);

// BufferedWriter
typedef CSTATUS WRITE_FUNCTION(
	void *Context,
	const char *Buffer,
	size_t Size,
	OUT size_t *ActualSize);
	
typedef struct _BUFFERED_WRITER {
	BUFFER Buffer;
	size_t DataSize;
	size_t BlockSize;
	WRITE_FUNCTION *WriteFunction;
	void *Context;
} BUFFERED_WRITER;

extern CSTATUS BufferedWriterInit(
	OUT BUFFERED_WRITER *Writer,
	size_t InitialAllocSize,
	size_t BlockSize,
	WRITE_FUNCTION *WriteFunction,
	void *Context);

extern void BufferedWriterUninit(
	BUFFERED_WRITER *Writer);

extern CSTATUS BufferedWriterFlush(
	BUFFERED_WRITER *Writer);

extern CSTATUS BufferedWriterWrite(
	BUFFERED_WRITER *Writer,
	const char *Buffer,
	size_t Size,
	OUT size_t *ActualSize);

extern CSTATUS BufferedWriterVPrintf(
	BUFFERED_WRITER *Writer,
	const char *Format,
	va_list ArgList);

extern CSTATUS BufferedWriterPrintf(
	BUFFERED_WRITER *Writer,
	const char *Format, ...);

// RadixTree
typedef struct _RADIX_TREE RADIX_TREE;

typedef struct _RADIX_TREE_ENTRY {
	RADIX_TREE *Tree;
	char *Token;
} RADIX_TREE_ENTRY;

#define RADIX_TREE_ENTRY_MAX (255)

struct _RADIX_TREE {
	void *Context;
	char *DeallocPointer;
	RADIX_TREE_ENTRY Entries[RADIX_TREE_ENTRY_MAX];
};

extern CSTATUS RadixTreeCreate(
	OUT RADIX_TREE **Tree);

extern void RadixTreeDestroy(
	RADIX_TREE *Tree);

extern CSTATUS RadixTreeInsert(
	RADIX_TREE *Tree,
	const char *Key,
	void *Context,
	int CaseInsensitive);

extern void *RadixTreeQuery(
	RADIX_TREE *Tree,
	const char *Key);

extern void *RadixTreeQueryCaseInsensitive(
	RADIX_TREE *Tree,
	const char *Key);

// Others
extern CSTATUS ErrnoToCStatus(
	int errno_);

extern CSTATUS LogVPrintf(
	const char *Format,
	va_list ArgList);

extern CSTATUS LogPrintf(
	const char *Format, ...);

extern CSTATUS Utf8ToUtf16GetSize(
	OUT size_t *SizeInChars,
	const char *Input);

extern CSTATUS Utf8ToUtf16(
	OUT wchar_t *Buffer,
	size_t BufferSizeInChars,
	const char *Input);

extern CSTATUS AnsiToUtf16GetSize(
	OUT size_t *SizeInChars,
	const char *Input);

extern CSTATUS AnsiToUtf16(
	OUT wchar_t *Buffer,
	size_t BufferSizeInChars,
	const char *Input);

#ifdef __cplusplus
}
#endif

#endif
