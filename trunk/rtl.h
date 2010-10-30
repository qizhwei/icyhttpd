#ifndef _RTL_H
#define _RTL_H

#include <stddef.h>
#include "list.h"
#include "win32.h"

#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(entry, type, field) \
	((type *)((char *)entry - (size_t)(&((type *)0)->field)))
#endif

typedef struct _RtlFifo RtlFifo;
typedef void RtlReadWriteCompletion(void *state, size_t size, int error);

extern void * RtlAllocateHeap(size_t size, char *tag);
extern int RtlReallocateHeap(void *pPointer, size_t newSize, char *tag);
extern void RtlFreeHeap(void *pointer);
extern void RtlTraceHeap(void);
extern char * RtlDuplicateString(const char *string);
extern int RtlCreatePipe(HANDLE *pipeRead, HANDLE *pipeWrite);
extern int RtlCreatePipeDuplex(HANDLE *pipeServer, HANDLE *pipeClient);
extern RtlFifo * RtlCreateFifo(void);
extern void RtlDestroyFifo(RtlFifo *fifo);
extern int RtlReadFifo(RtlFifo *fifo, char *buffer, size_t size, RtlReadWriteCompletion *completion, void *state);
extern int RtlWriteFifo(RtlFifo *fifo, char *buffer, size_t size, RtlReadWriteCompletion *completion, void *state);

#endif
