#ifndef _RTL_H
#define _RTL_H

#include <stddef.h>
#include "win32.h"

// Double linked list implementation

typedef struct _ListEntry {
	struct _ListEntry *Flink;
	struct _ListEntry *Blink;
} ListEntry;

#define InitializeListHead(head) \
	do { \
		(head)->Flink = (head); \
		(head)->Blink = (head); \
	} while (0)

#define InsertHeadList(head, entry) \
	do { \
		(entry)->Blink = (head); \
		(entry)->Flink = (head)->Flink; \
		(head)->Flink->Blink = (entry); \
		(head)->Flink = (entry); \
	} while (0)

#define InsertTailList(head, entry) \
	do { \
		(entry)->Blink = (head)->Blink; \
		(entry)->Flink = (head); \
		(head)->Blink->Flink = (entry); \
		(head)->Blink = (entry); \
	} while (0)

#define RemoveEntryList(entry) \
	do { \
		(entry)->Blink->Flink = (entry)->Flink; \
		(entry)->Flink->Blink = (entry)->Blink; \
	} while (0)

#define IsListEmpty(head) \
	((head)->Flink == (head))

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
extern int RtlCreatePipe(HANDLE *pipeServer, HANDLE *pipeClient);
extern RtlFifo * RtlCreateFifo(void);
extern void RtlDestroyFifo(RtlFifo *fifo);
extern int RtlReadFifo(RtlFifo *fifo, char *buffer, size_t size, RtlReadWriteCompletion *completion, void *state);
extern int RtlWriteFifo(RtlFifo *fifo, char *buffer, size_t size, RtlReadWriteCompletion *completion, void *state);

#endif
