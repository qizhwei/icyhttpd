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
}

namespace Httpd
{
	Dispatcher *Dispatcher::Instance()
	{
		static Dispatcher *d(new Dispatcher());
		return d;
	}

	Dispatcher::Dispatcher()
	{
		if ((dwMainFiberTls = TlsAlloc()) == TLS_OUT_OF_INDEXES
			|| (dwRecycleFiberTls = TlsAlloc()) == TLS_OUT_OF_INDEXES
			|| (dwDeletingFiberTls = TlsAlloc()) == TLS_OUT_OF_INDEXES
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
		LPVOID lpMainFiber;
		LPVOID lpRecycleFiber;

		if ((lpMainFiber = ConvertThreadToFiberEx(NULL, 0)) == NULL)
			throw FatalException();

		if ((lpRecycleFiber = CreateFiberEx(StackCommitSize, StackReserveSize, 0,
			&RecyclerCallback, disp)) == NULL)
			throw FatalException();

		TlsSetValue(disp->dwMainFiberTls, lpMainFiber);
		TlsSetValue(disp->dwRecycleFiberTls, lpRecycleFiber);

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
			} else if (CompletionKey == FiberDeleteKey) {
				DeleteFiber(lpOverlapped);
			} else if (CompletionKey == OverlappedOperationKey) {
				SwitchToFiber(static_cast<OverlappedOperation *>(lpOverlapped)->lpFiber);
			}
		}
	}
	
	VOID CALLBACK Dispatcher::RecyclerCallback(PVOID lpParameter)
	{
		Dispatcher *disp = reinterpret_cast<Dispatcher *>(lpParameter);

		while (true) {
			DeleteFiber(TlsGetValue(disp->dwDeletingFiberTls));
			SwitchToFiber(TlsGetValue(disp->dwMainFiberTls));
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

		// Switch to recycler to delete self
		Dispatcher *disp = Dispatcher::Instance();
		TlsSetValue(disp->dwDeletingFiberTls, GetCurrentFiber());
		SwitchToFiber(TlsGetValue(disp->dwRecycleFiberTls));
	}

	void Dispatcher::Queue(Callback *callback, void *param)
	{
		FiberCreateRecord *fcr = new FiberCreateRecord;
		LPVOID lpFiber;

		if ((lpFiber = CreateFiberEx(StackCommitSize, StackReserveSize, 0,
			&FiberCallback, reinterpret_cast<LPVOID>(fcr))) == NULL)
		{
			delete fcr;
			throw SystemException();
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
			throw SystemException();
	}

	void Dispatcher::Block()
	{
		SwitchToFiber(TlsGetValue(this->dwMainFiberTls));
	}
}
