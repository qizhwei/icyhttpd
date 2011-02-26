#include "Dispatcher.h"
#include "Exception.h"
#include "Types.h"
#include "Constant.h"
#include <cstdio>

namespace
{
	using namespace Httpd;

	struct FiberCreateRecord
	{
		Callback *callback;
		void *param;
	};

	struct ThreadData
	{
		LPVOID lpMainFiber;
		LPVOID lpDeletingFiber;
		HANDLE hReadyEvent;
	};
}

namespace Httpd
{
	Dispatcher &Dispatcher::Instance()
	{
		static Dispatcher *d(new Dispatcher());
		return *d;
	}

	Dispatcher::Dispatcher()
	{
		if ((dwTlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES
			|| (hQueue = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
			throw FatalException();

		for (int i = 0; i < ThreadCount; ++i) {
			HANDLE hThread = CreateThread(NULL, 0, &ThreadCallback, this, 0, NULL);
			if (hThread == NULL)
				throw FatalException();
			CloseHandle(hThread);
		}
	}

	DWORD WINAPI Dispatcher::ThreadCallback(LPVOID lpParameter)
	{
		// The dispatcher class may be partially constructed since that
		// the thread creation was in the constructor

		Dispatcher *disp = reinterpret_cast<Dispatcher *>(lpParameter);
		ThreadData threadData;

		if ((threadData.lpMainFiber = ConvertThreadToFiberEx(NULL, 0)) == NULL)
			throw FatalException();
		threadData.lpDeletingFiber = NULL;
		threadData.hReadyEvent = NULL;

		TlsSetValue(disp->dwTlsIndex, &threadData);

		DWORD NumberOfBytes;
		ULONG_PTR CompletionKey;
		LPOVERLAPPED lpOverlapped;
		BOOL bSuccess;
		
		while (true) {
			bSuccess = GetQueuedCompletionStatus(disp->hQueue,
				&NumberOfBytes, &CompletionKey, &lpOverlapped, INFINITE);

			if (bSuccess == FALSE && lpOverlapped == NULL)
				throw FatalException();

			if (CompletionKey == FiberCreateKey) {
				SwitchToFiber(lpOverlapped);
			} else if (CompletionKey == OverlappedOperationKey) {
				OverlappedOperation *overlapped = static_cast<OverlappedOperation *>(lpOverlapped);
				WaitForSingleObject(overlapped->hReadyEvent, INFINITE);
				SwitchToFiber(overlapped->lpFiber);
			}

			if (threadData.lpDeletingFiber != NULL) {
				DeleteFiber(threadData.lpDeletingFiber);
				threadData.lpDeletingFiber = NULL;
			} else if (threadData.hReadyEvent != NULL) {
				SetEvent(threadData.hReadyEvent);
				threadData.hReadyEvent = NULL;
			}
		}
	}
	
	VOID CALLBACK Dispatcher::FiberCallback(PVOID lpParameter)
	{
		FiberCreateRecord *fcr = reinterpret_cast<FiberCreateRecord *>(lpParameter);
		Callback *callback = fcr->callback;
		void *param = fcr->param;
		delete fcr;

		// Perform callback
		callback(param);

		// Switch back and delete self
		ThreadData &threadData = *static_cast<ThreadData *>(TlsGetValue(Dispatcher::Instance().dwTlsIndex));
		threadData.lpDeletingFiber = GetCurrentFiber();
		SwitchToFiber(threadData.lpMainFiber);
	}

	void Dispatcher::Queue(Callback *callback, void *param)
	{
		FiberCreateRecord *fcr = new FiberCreateRecord;
		LPVOID lpFiber;

		if ((lpFiber = CreateFiberEx(StackCommitSize, StackReserveSize, 0,
			&FiberCallback, reinterpret_cast<LPVOID>(fcr))) == NULL)
		{
			delete fcr;
			throw Exception();
		}

		fcr->callback = callback;
		fcr->param = param;

		if (!PostQueuedCompletionStatus(this->hQueue, 0, FiberCreateKey,
			reinterpret_cast<LPOVERLAPPED>(lpFiber)))
			throw FatalException();
	}

	void Dispatcher::BindHandle(HANDLE hFile, ULONG_PTR key)
	{
		if (CreateIoCompletionPort(hFile, this->hQueue, key, 0) == NULL)
			throw Exception();
	}

	UInt32 Dispatcher::Block(HANDLE hObject, OverlappedOperation &overlapped)
	{
		ThreadData &threadData = *static_cast<ThreadData *>(TlsGetValue(this->dwTlsIndex));
		threadData.hReadyEvent = overlapped.hReadyEvent;
		SwitchToFiber(threadData.lpMainFiber);

		DWORD dwBytesTransferred;
		if (!GetOverlappedResult(hObject, &overlapped, &dwBytesTransferred, FALSE)) {
			if (GetLastError() == ERROR_HANDLE_EOF)
				return 0;
			else
				throw Exception();
		}

		return dwBytesTransferred;
	}
}
