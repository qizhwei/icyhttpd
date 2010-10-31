#include "rtl.h"
#include <stdlib.h>

#ifdef TRACE_ALLOC
#include <stdio.h>

typedef struct _RtlpHeapTrace {
	char *Tag;
	ListEntry Entry;
} RtlpHeapTrace;

static ListEntry RtlpHeapTraceList = {&RtlpHeapTraceList, &RtlpHeapTraceList};

#endif

void * RtlAllocateHeap(size_t size, char *tag)
{
	void *result;
	
#ifdef TRACE_ALLOC
	RtlpHeapTrace *trace;
	size += sizeof(RtlpHeapTrace);
#endif

	result = malloc(size);
	
#ifdef TRACE_ALLOC
	if (result != NULL) {
		trace = result;
		trace->Tag = tag;
		InsertTailList(&RtlpHeapTraceList, &trace->Entry);
		result = (unsigned char *)result + sizeof(RtlpHeapTrace);
	}
#endif

	return result;
}

int RtlReallocateHeap(void *pPointer, size_t newSize, char *tag)
{
	void *old = *(void **)pPointer;
	void *result;
	
#ifdef TRACE_ALLOC
	RtlpHeapTrace *trace;
	if (old != NULL) {
		old = (unsigned char *)old - sizeof(RtlpHeapTrace);
		trace = old;
		RemoveEntryList(&trace->Entry);
	}
	newSize += sizeof(RtlpHeapTrace);
#endif

	result = realloc(old, newSize);
	if (result == NULL) {
		return 1;
	}
	
#ifdef TRACE_ALLOC
	trace = result;
	trace->Tag = tag;
	InsertTailList(&RtlpHeapTraceList, &trace->Entry);
	result = (unsigned char *)result + sizeof(RtlpHeapTrace);
#endif
	
	*(void **)pPointer = result;
	return 0;
}

void RtlFreeHeap(void *pointer)
{
#ifdef TRACE_ALLOC
	RtlpHeapTrace *trace;
	
	if (pointer != NULL) {
		pointer = (unsigned char *)pointer - sizeof(RtlpHeapTrace);
		trace = pointer;
		RemoveEntryList(&trace->Entry);
	}
#endif

	free(pointer);
}

void RtlTraceHeap(void)
{
#ifdef TRACE_ALLOC
	ListEntry *entry;
	RtlpHeapTrace *trace;
	char *tagWord;
	
	for (entry = RtlpHeapTraceList.Flink; entry != &RtlpHeapTraceList; entry = entry->Flink) {
		trace = CONTAINING_RECORD(entry, RtlpHeapTrace, Entry);
		printf("%s\n", trace->Tag);
	}
#endif
}
