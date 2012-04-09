#include <stdlib.h>
#include <httpd_util.h>
#include "fiberio.h"

int IopThreadCount;
DWORD IopTlsIndex;
HANDLE IopQueueHandle;
IOP_THREAD_BLOCK *IopThreadBlocks;

static void IopDispatcherAbort(CSTATUS status)
{
	LogPrintf("IopDispatcherAbort() called with status %d\n", status);
	abort();
}

static CSTATUS IopNullDelayedIo(IOP_IO_BLOCK *IoBlock)
{
	return C_SUCCESS;
}

DWORD CALLBACK IopDispatcherThreadEntry(
	LPVOID Context)
{
	IOP_THREAD_BLOCK *threadBlock = (IOP_THREAD_BLOCK *)Context;
	IOP_IO_BLOCK nullIoBlock = {0};
	IOP_IO_BLOCK *delayedIoBlock;
	DWORD numberOfBytes;
	ULONG_PTR completionKey;
	LPOVERLAPPED lpOverlapped;
	BOOL bSuccess;
	DWORD dwErrorCode;

	threadBlock->MainFiberHandle = ConvertThreadToFiber(NULL);
	if (!threadBlock->MainFiberHandle)
		IopDispatcherAbort(Win32ErrorCodeToCStatus(GetLastError()));

	if (!TlsSetValue(IopTlsIndex, threadBlock))
		IopDispatcherAbort(Win32ErrorCodeToCStatus(GetLastError()));

	threadBlock->DelayedIoStatus = C_SUCCESS;
	nullIoBlock.DelayedIoFunction = IopNullDelayedIo;

	while (1) {
		threadBlock->DelayedIoBlock = &nullIoBlock;

		bSuccess = GetQueuedCompletionStatus(IopQueueHandle,
			&numberOfBytes, &completionKey, &lpOverlapped, INFINITE);

		if (!bSuccess && !lpOverlapped) {
			dwErrorCode = GetLastError();
			if (dwErrorCode == ERROR_ABANDONED_WAIT_0)
				return 0;

			IopDispatcherAbort(Win32ErrorCodeToCStatus(dwErrorCode));
		}

		if (completionKey == FIBERIO_SWITCH_KEY) {
			// lpOverlapped contains the pointer to the fiber
			SwitchToFiber((LPVOID)lpOverlapped);
		} else if (completionKey == FIBERIO_COMPLETE_KEY) {
			// lpOverlapped contains a record of type IOP_IO_BLOCK
			SwitchToFiber(CONTAINING_RECORD(lpOverlapped, IOP_IO_BLOCK, Overlapped)->FiberHandle);
		}

		// SwitchToFiber(threadBlock->MainFiberHandle) usually goes here

		while (1) {
			delayedIoBlock = threadBlock->DelayedIoBlock;
			threadBlock->DelayedIoStatus = delayedIoBlock->DelayedIoFunction(delayedIoBlock);
			if (SUCCESS(threadBlock->DelayedIoStatus))
				break;
			SwitchToFiber(delayedIoBlock->FiberHandle);
		}
	}
}

CSTATUS IopDispatcherBlock(
	IOP_IO_BLOCK *IoBlock,
	HANDLE ObjectHandle,
	OUT size_t *ActualSize)
{
	IOP_THREAD_BLOCK *threadBlock = CURRENT_THREAD_BLOCK();
	DWORD dwBytesTransferred;

	threadBlock->DelayedIoBlock = IoBlock;
	SwitchToFiber(threadBlock->MainFiberHandle);

	// N.B. Thread block may be changed
	threadBlock = CURRENT_THREAD_BLOCK();
	if (!SUCCESS(threadBlock->DelayedIoStatus))
		return threadBlock->DelayedIoStatus;

	if (!GetOverlappedResult(ObjectHandle, &IoBlock->Overlapped,
		&dwBytesTransferred, FALSE))
		return Win32ErrorCodeToCStatus(GetLastError());

	*ActualSize = (size_t)dwBytesTransferred;
	return C_SUCCESS;
}
