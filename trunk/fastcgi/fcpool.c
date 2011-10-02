#include "fcp.h"
#include "ob.h"
#include <assert.h>

static void CALLBACK FcpProcessTimerProc(void *state, DWORD dwTimerLowValue, DWORD dwTimerHighValue);

FcPool * FcCreatePool(const char *name, const char *commandLine, int idleTime, int maxRequests)
{
	FcPool *pool;
	
	// Allocate memory for the object
	pool = (FcPool *)ObCreateObject(&FcpPoolObjectType, sizeof(FcPool), FcpPoolDirectory, name);
	if (pool == NULL) {
		return NULL;
	}
	
	// Initialize the object
	InitializeListHead(&pool->RunningList);
	InitializeListHead(&pool->PoolingList);
	pool->CommandLine = RtlDuplicateString(commandLine);
	
	if (pool->CommandLine == NULL) {
		ObDereferenceObject(pool);
		return NULL;
	}
	
	pool->IdleTime = idleTime;
	pool->MaxRequests = maxRequests;
	
	return pool;
}

int FcpPushPoolProcess(FcpProcess *process)
{
	FcPool *pool = process->Pool;
	FcpWaitBlock *waitBlock;
	HANDLE timer;
	LARGE_INTEGER dueTime;
	
	assert(process->State == FCP_STATE_READY);
	
	// Allocate memory for the wait block
	waitBlock = (FcpWaitBlock *)ObCreateObject(&FcpWaitBlockObjectType, sizeof(FcpWaitBlock), NULL, NULL);
	if (waitBlock == NULL) {
		return 1;
	}
	
	// Create a waitable timer
	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	if (timer == NULL) {
		ObDereferenceObject(waitBlock);
		return 1;
	}
	
	// Set the waitable timer
	dueTime.QuadPart = -10000LL * pool->IdleTime;
	if (!SetWaitableTimer(timer, &dueTime, 0, &FcpProcessTimerProc, waitBlock, FALSE)) {
		ObDereferenceObject(waitBlock);
		CloseHandle(timer);
		return 1;
	}

	// Initialize the wait block
	waitBlock->Process = process;
	waitBlock->Timer = timer;
	waitBlock->Cancelled = 0;
	
	// Assign the wait block to the process
	process->State = FCP_STATE_POOLING;
	process->WaitBlock = (FcpWaitBlock *)ObReferenceObjectByPointer(waitBlock, NULL);
	
	// Switch the process to the pooling list
	RemoveEntryList(&process->PoolEntry);
	InsertHeadList(&pool->PoolingList, &process->PoolEntry);
	
	// Dereference the process
	ObDereferenceObject(process);
	
	return 0;
}

void FcpRemovePoolProcess(FcpProcess *process)
{
	FcpWaitBlock *waitBlock = process->WaitBlock;
	assert(process->State = FCP_STATE_POOLING);
	
	// Cancel the waitable timer
	if (CancelWaitableTimer(waitBlock->Timer)) {
		ObDereferenceObject(waitBlock);
	} else {
		waitBlock->Cancelled = 1;
	}
	
	// Switch the process to the running list
	RemoveEntryList(&process->PoolEntry);
	InsertHeadList(&process->Pool->RunningList, &process->PoolEntry);
	process->WaitBlock = NULL;
	process->State = FCP_STATE_READY;
	ObDereferenceObject(waitBlock);
}

FcpProcess * FcpPopPoolProcess(FcPool *pool)
{
	FcpProcess *process;
	
	if (IsListEmpty(&pool->PoolingList)) {
		return NULL;
	}
	
	process = CONTAINING_RECORD(pool->PoolingList.Flink, FcpProcess, PoolEntry);
	FcpRemovePoolProcess(process);
	return ObReferenceObjectByPointer(process, NULL);
}

static void CALLBACK FcpProcessTimerProc(void *state, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	FcpWaitBlock *waitBlock = state;
	FcpProcess *process;
	
	if (!waitBlock->Cancelled) {
		process = waitBlock->Process;
		assert(process->State == FCP_STATE_POOLING);
		RemoveEntryList(&process->PoolEntry);
		InsertHeadList(&process->Pool->RunningList, &process->PoolEntry);
		process->WaitBlock = NULL;
		process->State = FCP_STATE_READY;
		FcpTerminateProcess(process, 0);

		// Dereference for the process
		ObDereferenceObject(waitBlock);
	}
	
	// Dereference for the callback routine
	ObDereferenceObject(waitBlock);
}

void FcpWaitBlockClose(void *object)
{
	FcpWaitBlock *waitBlock = object;
	
	if (waitBlock->Timer != NULL) {
		CloseHandle(waitBlock->Timer);
	}
}
