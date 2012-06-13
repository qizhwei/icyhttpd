#include <stdlib.h>
#include <assert.h>
#include "fiberio.h"

static CSTATUS DeleteFiberCallback(
	IOP_IO_BLOCK *IoBlock)
{
	DeleteFiber(IoBlock->FiberHandle);
	return C_SUCCESS;
}

static void IopThreadExit(
	IO_THREAD *Thread)
{
	IOP_IO_BLOCK deleteFiberBlock;
	IOP_THREAD_BLOCK *threadBlock = CURRENT_THREAD_BLOCK();
	IO_THREAD *joinThread;

	// Wake up joiners
	joinThread = (IO_THREAD *)InterlockedCompareExchangePointer(&Thread->JoinThread, Thread, NULL);
	if (joinThread != NULL) {
		PostQueuedCompletionStatus(IopQueueHandle, 0, FIBERIO_SWITCH_KEY,
			(LPOVERLAPPED)joinThread->FiberHandle);
	}

	IoDetachThread(Thread);

	// Use delayed I/O to delete currently executing fiber
	deleteFiberBlock.FiberHandle = GetCurrentFiber();
	deleteFiberBlock.DelayedIoFunction = DeleteFiberCallback;
	threadBlock->DelayedIoBlock = &deleteFiberBlock;
	SwitchToFiber(threadBlock->MainFiberHandle);
}

static void CALLBACK IopUserFiberEntry(
	LPVOID Context)
{
	IO_THREAD *thread = (IO_THREAD *)Context;
	thread->ThreadEntry(thread->UserContext);
	IopThreadExit(thread);
	assert(!"never be here");
}

CSTATUS IoCreateThread(
	OUT IO_THREAD **Thread,
	IO_THREAD_ENTRY *ThreadEntry,
	void *Context)
{
	IO_THREAD *object;

	object = (IO_THREAD *)malloc(sizeof(IO_THREAD));
	if (object == NULL) {
		return C_BAD_ALLOC;
	}

	// One for user, one for the thread's execution
	object->ReferenceCount = 2;

	object->ThreadEntry = ThreadEntry;
	object->UserContext = Context;

	object->FiberHandle = CreateFiberEx(
		FIBERIO_FIBER_STACK_COMMIT_SIZE,
		FIBERIO_FIBER_STACK_RESERVE_SIZE,
		0, IopUserFiberEntry, object);

	if (object->FiberHandle == NULL) {
		free(object);
		return Win32ErrorCodeToCStatus(GetLastError());
	}

	object->JoinThread = NULL;

	if (!PostQueuedCompletionStatus(IopQueueHandle, 0,
		FIBERIO_SWITCH_KEY, (LPOVERLAPPED)object->FiberHandle))
	{
		DeleteFiber(object->FiberHandle);
		free(object);
		return Win32ErrorCodeToCStatus(GetLastError());
	}

	*Thread = object;
	return C_SUCCESS;
}

void IoDetachThread(
	IO_THREAD *Thread)
{
	assert(Thread->ReferenceCount > 0);

	if (InterlockedDecrement(&Thread->ReferenceCount) == 0)
		free(Thread);
}

CSTATUS IoJoinThread(
	IO_THREAD *Thread)
{
	IO_THREAD *CurrentThread = (IO_THREAD *)GetFiberData();
	
	if (!InterlockedCompareExchangePointer(&Thread->JoinThread, CurrentThread, NULL))
		SwitchToFiber(CURRENT_THREAD_BLOCK()->MainFiberHandle);

	IoDetachThread(Thread);
	return C_SUCCESS;
}

static void CALLBACK IopApcEntry(
	ULONG_PTR Context)
{
	IOP_APC_BLOCK *block = (IOP_APC_BLOCK *)Context;
	block->ApcEntry(block->Context, (IO_APC *)&block->IoBlock.Overlapped);
}

static CSTATUS IoInvokeApcDelayedIo(
	IOP_IO_BLOCK *IoBlock)
{
	IOP_APC_BLOCK *block = CONTAINING_RECORD(IoBlock, IOP_APC_BLOCK, IoBlock);

	if (!QueueUserAPC(IopApcEntry, IopApcThread, (ULONG_PTR)block))
		return Win32ErrorCodeToCStatus(GetLastError());

	return C_SUCCESS;
}

CSTATUS IoInvokeApc(
	IO_APC_ENTRY *ApcEntry,
	void *Context)
{
	IOP_APC_BLOCK block;
	IOP_THREAD_BLOCK *threadBlock = CURRENT_THREAD_BLOCK();

	INIT_IO_BLOCK(&block.IoBlock, IoInvokeApcDelayedIo);
	block.ApcEntry = ApcEntry;
	block.Context = Context;
	threadBlock->DelayedIoBlock = &block.IoBlock;
	SwitchToFiber(threadBlock->MainFiberHandle);

	// N.B. Thread block may be changed
	threadBlock = CURRENT_THREAD_BLOCK();
	return threadBlock->DelayedIoStatus;
}

void IoCompleteApc(
	IO_APC *Apc)
{
	if (!PostQueuedCompletionStatus(IopQueueHandle, 0, FIBERIO_COMPLETE_KEY, (LPOVERLAPPED)Apc))
		abort();
}
