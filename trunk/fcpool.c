#include "fcp.h"
#include <assert.h>

static void CALLBACK FcpProcessTimerProc(void *state, DWORD dwTimerLowValue, DWORD dwTimerHighValue);

FcPool * FcCreatePool(const char *commandLine, int idleTime, int maxRequests)
{
	FcPool *pool;
	
	// Perform global initialization
	if (FcpInitialize() != 0) {
		return NULL;
	}
	
	// Allocate memory for the object
	pool = RtlAllocateHeap(sizeof(FcPool), "FcCreatePool");
	if (pool == NULL) {
		return NULL;
	}
	
	// Initialize the object
	pool->ReferenceCount = 1;
	InitializeListHead(&pool->RunningList);
	InitializeListHead(&pool->PoolingList);
	pool->CommandLine = RtlDuplicateString(commandLine);
	
	if (pool->CommandLine == NULL) {
		RtlFreeHeap(pool);
		return NULL;
	}
	
	pool->IdleTime = idleTime;
	pool->MaxRequests = maxRequests;
	
	return pool;
}

void FcpDereferencePool(FcPool *pool)
{
	assert(pool->ReferenceCount > 0);
	
	pool->ReferenceCount -= 1;
	// TODO: Not implemented
}

int FcpPushPoolProcess(FcProcess *process)
{
	FcPool *pool = process->Pool;
	FcpWaitBlock *waitBlock;
	HANDLE timer;
	LARGE_INTEGER dueTime;
	
	assert(process->State == FCP_STATE_READY);
	
	// Allocate memory for the wait block
	waitBlock = RtlAllocateHeap(sizeof(FcpWaitBlock), "FcpPushPoolProcess");
	if (waitBlock == NULL) {
		return 1;
	}
	
	// Create a waitable timer
	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	if (timer == NULL) {
		RtlFreeHeap(waitBlock);
		return 1;
	}
	
	// Set the waitable timer
	dueTime.QuadPart = -10000LL * pool->IdleTime;
	if (!SetWaitableTimer(timer, &dueTime, 0, &FcpProcessTimerProc, waitBlock, FALSE)) {
		RtlFreeHeap(waitBlock);
		CloseHandle(timer);
		return 1;
	}

	// Initialize the wait block
	waitBlock->ReferenceCount = 2;
	waitBlock->Process = process;
	waitBlock->Timer = timer;
	waitBlock->Cancelled = 0;
	
	// Assign the wait block to the process
	process->State = FCP_STATE_POOLING;
	process->WaitBlock = waitBlock;
	
	// Switch the process to the pooling list
	RemoveEntryList(&process->PoolEntry);
	InsertHeadList(&pool->PoolingList, &process->PoolEntry);
	
	return 0;
}

void FcpRemovePoolProcess(FcProcess *process)
{
	FcpWaitBlock *waitBlock = process->WaitBlock;
	assert(process->State = FCP_STATE_POOLING);
	
	// Cancel the waitable timer
	if (CancelWaitableTimer(waitBlock->Timer)) {
		FcpDereferenceWaitBlock(waitBlock);
	} else {
		waitBlock->Cancelled = 1;
	}
	
	// Switch the process to the running list
	RemoveEntryList(&process->PoolEntry);
	InsertHeadList(&process->Pool->RunningList, &process->PoolEntry);
	process->WaitBlock = NULL;
	process->State = FCP_STATE_READY;
	FcpDereferenceWaitBlock(waitBlock);
}

FcProcess * FcpPopPoolProcess(FcPool *pool)
{
	FcProcess *process;
	
	if (IsListEmpty(&pool->PoolingList)) {
		return NULL;
	}
	
	process = CONTAINING_RECORD(pool->PoolingList.Flink, FcProcess, PoolEntry);
	FcpRemovePoolProcess(process);
	return process;
}

static void CALLBACK FcpProcessTimerProc(void *state, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	FcpWaitBlock *waitBlock = state;
	FcProcess *process;
	
	if (!waitBlock->Cancelled) {
		process = waitBlock->Process;
		assert(process->State == FCP_STATE_POOLING);
		RemoveEntryList(&process->PoolEntry);
		InsertHeadList(&process->Pool->RunningList, &process->PoolEntry);
		process->WaitBlock = NULL;
		process->State = FCP_STATE_READY;
		FcpTerminateProcess(process, 0);

		// Dereference for the process
		FcpDereferenceWaitBlock(waitBlock);
	}
	
	// Dereference for the callback routine
	FcpDereferenceWaitBlock(waitBlock);
}

void FcpDereferenceWaitBlock(FcpWaitBlock *waitBlock)
{
	assert(waitBlock->ReferenceCount > 0);

	waitBlock->ReferenceCount -= 1;
	if (waitBlock->ReferenceCount == 0) {
		CloseHandle(waitBlock->Timer);
		RtlFreeHeap(waitBlock);
	}
}
