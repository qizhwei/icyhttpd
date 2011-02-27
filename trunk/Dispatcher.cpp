#include "Dispatcher.h"
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
		Operation *operation;
		bool failed;
	};

	struct DeleteFiberOperation: public Operation
	{
		DeleteFiberOperation()
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

	struct NullOperation: public Operation
	{
		NullOperation()
			: Operation(NULL)
		{}

		virtual bool operator()()
		{
			return true;
		}

	} nullOperation;
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

		if ((threadData.lpMainFiber = ConvertThreadToFiber(NULL)) == NULL)
			throw FatalException();
		threadData.operation = &nullOperation;
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
			} else if (CompletionKey == OverlappedOperationKey) {
				OverlappedOperation *overlapped = static_cast<OverlappedOperation *>(lpOverlapped);
				overlapped->SwitchBack();
			}

			while (!(*threadData.operation)()) {
				threadData.failed = true;
				threadData.operation->SwitchBack();
			}

			threadData.operation = &nullOperation;
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
		DeleteFiberOperation operation;
		threadData.operation = &operation;
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

	Int32 Dispatcher::Block(HANDLE hObject, OverlappedOperation &operation)
	{
		ThreadData &threadData = *static_cast<ThreadData *>(TlsGetValue(this->dwTlsIndex));
		threadData.operation = &operation;
		SwitchToFiber(threadData.lpMainFiber);

		if (threadData.failed)
			return -1;

		DWORD dwBytesTransferred;
		if (!GetOverlappedResult(hObject, &operation, &dwBytesTransferred, FALSE))
			return (GetLastError() == ERROR_HANDLE_EOF) ? 0 : -1;

		return static_cast<Int32>(dwBytesTransferred);
	}
}
