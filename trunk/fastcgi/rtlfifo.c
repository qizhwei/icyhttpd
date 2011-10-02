#include "rtl.h"
#include <string.h>

typedef struct _RtlpFifoBlock {
	ListEntry Entry;
	char *Buffer;
	size_t Size;
	size_t Transferred;
	int Error;
	RtlIoCompletion *Completion;
	void *CompletionState;
} RtlpFifoBlock;

struct _RtlFifo {
	int EndOfFile;
	ListEntry ReadList;
	ListEntry WriteList;
};

static void RtlpTransferFifo(RtlFifo *fifo);
static void RtlpCompleteFifoBlock(RtlpFifoBlock *block);
static void RtlpCancelFifo(RtlFifo *fifo);

RtlFifo * RtlCreateFifo(void)
{
	RtlFifo *fifo = (RtlFifo *)RtlAllocateHeap(sizeof(RtlFifo));
	
	if (fifo == NULL) {
		return NULL;
	}
	
	fifo->EndOfFile = 0;
	InitializeListHead(&fifo->ReadList);
	InitializeListHead(&fifo->WriteList);
	return fifo;
}

void RtlDestroyFifo(RtlFifo *fifo)
{
	RtlpCancelFifo(fifo);
	RtlFreeHeap(fifo);
}

int RtlReadFifo(RtlFifo *fifo, char *buffer, size_t size, RtlIoCompletion *completion, void *state)
{
	RtlpFifoBlock *block;
	
	if (size == 0 || fifo->EndOfFile) {
		return 1;
	}
	
	block = RtlAllocateHeap(sizeof(RtlpFifoBlock));
	if (block == NULL) {
		return 1;
	}
	
	InsertTailList(&fifo->ReadList, &block->Entry);
	block->Buffer = buffer;
	block->Size = size;
	block->Transferred = 0;
	block->Error = 0;
	block->Completion = completion;
	block->CompletionState = state;
	
	RtlpTransferFifo(fifo);
	
	return 0;
}

int RtlWriteFifo(RtlFifo *fifo, char *buffer, size_t size, RtlIoCompletion *completion, void *state)
{
	RtlpFifoBlock *block;
	
	if (fifo->EndOfFile) {
		return 1;
	}
	
	block = RtlAllocateHeap(sizeof(RtlpFifoBlock));
	if (block == NULL) {
		return 1;
	}
	
	InsertTailList(&fifo->WriteList, &block->Entry);
	block->Buffer = buffer;
	block->Size = size;
	block->Transferred = 0;
	block->Error = 0;
	block->Completion = completion;
	block->CompletionState = state;
	
	RtlpTransferFifo(fifo);
	
	return 0;
}

static void RtlpTransferFifo(RtlFifo *fifo)
{
	RtlpFifoBlock *readBlock, *writeBlock;
	size_t readLength, writeLength;
	
	while (!IsListEmpty(&fifo->ReadList) && !IsListEmpty(&fifo->WriteList)) {
		readBlock = CONTAINING_RECORD(fifo->ReadList.Flink, RtlpFifoBlock, Entry);
		writeBlock = CONTAINING_RECORD(fifo->WriteList.Flink, RtlpFifoBlock, Entry);
		
		if (writeBlock->Size == 0) {
			RtlpCompleteFifoBlock(readBlock);
			RtlpCompleteFifoBlock(writeBlock);
			RtlpCancelFifo(fifo);
			fifo->EndOfFile = 1;
		} else {
			readLength = readBlock->Size - readBlock->Transferred;
			writeLength = writeBlock->Size - writeBlock->Transferred;

			if (readLength > writeLength) {
				readLength = writeLength;
			}
			
			memcpy(&readBlock->Buffer[readBlock->Transferred],
				&writeBlock->Buffer[writeBlock->Transferred], readLength);
			readBlock->Transferred += readLength;
			writeBlock->Transferred += readLength;
			
			if (readBlock->Size == readBlock->Transferred) {
				RtlpCompleteFifoBlock(readBlock);
			}
			
			if (writeBlock->Size == writeBlock->Transferred) {
				RtlpCompleteFifoBlock(writeBlock);
			}
		}
	}
}

static void CALLBACK RtlpCompleteFifoApc(ULONG_PTR dwParam)
{
	RtlpFifoBlock *block = (RtlpFifoBlock *)dwParam;
	block->Completion(block->CompletionState, block->Transferred, block->Error);
	RtlFreeHeap(block);
}

static void RtlpCompleteFifoBlock(RtlpFifoBlock *block)
{
	RemoveEntryList(&block->Entry);
	if (!QueueUserAPC(&RtlpCompleteFifoApc, GetCurrentThread(), (ULONG_PTR)block)) {
		block->Completion(block->CompletionState, block->Transferred, 1);
		RtlFreeHeap(block);
	}
}

static void RtlpCancelFifo(RtlFifo *fifo)
{
	RtlpFifoBlock *block;
	
	while (!IsListEmpty(&fifo->ReadList)) {
		block = CONTAINING_RECORD(fifo->ReadList.Flink, RtlpFifoBlock, Entry);
		block->Error = 1;
		RtlpCompleteFifoBlock(block);
	}
	
	while (!IsListEmpty(&fifo->WriteList)) {
		block = CONTAINING_RECORD(fifo->WriteList.Flink, RtlpFifoBlock, Entry);
		block->Error = 1;
		RtlpCompleteFifoBlock(block);
	}
}
