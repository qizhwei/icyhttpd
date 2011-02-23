#include "Dispatcher.h"
#include "Types.h"
#include <cstdio>

namespace
{
	using namespace Httpd;

	const SIZE_T StackCommitSize = 4096;
	const SIZE_T StackReserveSize = 16384;
	const int ThreadCount = 4;

	enum CompletionKey {
		FiberCreateKey,
		FiberDeleteKey,
		// Win32OverlapKey,
	};

	struct FiberCreateRecord {
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
		if ((dwMainFiberTls = TlsAlloc()) == TLS_OUT_OF_INDEXES)
			throw FatalException();

		if ((hQueue = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
			throw FatalException();

		for (int i = 0; i < ThreadCount; ++i) {
			HANDLE hThread = CreateThread(NULL, 0, &ThreadCallback, reinterpret_cast<LPVOID>(this), 0, NULL);
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
		LPVOID lpFiber;

		if ((lpFiber = ConvertThreadToFiberEx(NULL, 0)) == NULL)
			throw FatalException();

		if (!TlsSetValue(disp->dwMainFiberTls, lpFiber))
			throw FatalException();

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
				SwitchToFiber(reinterpret_cast<LPVOID>(lpOverlapped));
			} else if (CompletionKey == FiberDeleteKey) {
				DeleteFiber(reinterpret_cast<LPVOID>(lpOverlapped));
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

		// Switch back and delete fiber
		Dispatcher *disp = Dispatcher::Instance();
		if (!PostQueuedCompletionStatus(disp->hQueue, 0, FiberDeleteKey,
			reinterpret_cast<LPOVERLAPPED>(GetCurrentFiber())))
			throw FatalException();
		disp->SwitchBack();
	}

	void Dispatcher::Queue(Callback *callback, void *param)
	{
		FiberCreateRecord *fcr = new FiberCreateRecord;
		LPVOID lpFiber;

		if ((lpFiber = CreateFiberEx(StackCommitSize, StackReserveSize, 0,
			&FiberCallback, reinterpret_cast<LPVOID>(fcr))) == NULL)
		{
			delete fcr;
			throw ResourceInsufficientException();
		}

		fcr->callback = callback;
		fcr->param = param;

		if (!PostQueuedCompletionStatus(this->hQueue, 0, FiberCreateKey,
			reinterpret_cast<LPOVERLAPPED>(lpFiber)))
			throw FatalException();
	}

	void Dispatcher::SwitchBack()
	{
		LPVOID lpFiber = TlsGetValue(this->dwMainFiberTls);
		SwitchToFiber(lpFiber);
	}
}
