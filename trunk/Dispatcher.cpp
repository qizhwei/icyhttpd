#include "Dispatcher.h"
#include "Utility.h"
#include "Exception.h"
#include "Types.h"
#include "Constant.h"

using namespace Httpd;

namespace
{
	struct FiberCreateRecord
	{
		Callback *callback;
		void *param;
	};

	struct ThreadData
	{
		LPVOID lpMainFiber;
		Completion *completion;
		bool failed;
	};

	struct DeleteFiberCompletion: public Completion
	{
		DeleteFiberCompletion()
			: lpFiber(GetCurrentFiber())
		{}

		virtual bool operator()()
		{
			DeleteFiber(this->lpFiber);
			// This object is allocated on stack and no longer exist after fiber deletion

			return true;
		}

		LPVOID lpFiber;
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

		for (int i = 0; i < DispatcherThreadCount; ++i) {
			HANDLE hThread = CreateThread(NULL, StackReserveSize, &ThreadCallback, this, 0, NULL);
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
		Completion nullCompletion;

		if ((threadData.lpMainFiber = ConvertThreadToFiber(NULL)) == NULL)
			throw FatalException();
		threadData.completion = &nullCompletion;
		threadData.failed = false;

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
			} else if (CompletionKey == OverlappedCompletionKey) {
				OverlappedCompletion *overlapped = static_cast<OverlappedCompletion *>(lpOverlapped);
				overlapped->SwitchBack();
			}

			while (!(*threadData.completion)()) {
				threadData.failed = true;
				threadData.completion->SwitchBack();
			}

			threadData.completion = &nullCompletion;
			threadData.failed = false;
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
		DeleteFiberCompletion dfc;
		threadData.completion = &dfc;
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

	UInt32 Dispatcher::Block(HANDLE hObject, OverlappedCompletion &oc)
	{
		ThreadData &threadData = *static_cast<ThreadData *>(TlsGetValue(this->dwTlsIndex));
		threadData.completion = &oc;
		SwitchToFiber(threadData.lpMainFiber);

		if (threadData.failed)
			throw SystemException();

		DWORD dwBytesTransferred;
		if (!GetOverlappedResult(hObject, &oc, &dwBytesTransferred, FALSE)) {
			if (GetLastError() == ERROR_HANDLE_EOF)
				return 0;
			else
				throw SystemException();
		}

		return dwBytesTransferred;
	}
}
