#include "Dispatcher.h"
#include "Utility.h"
#include "Exception.h"
#include "Types.h"
#include "Constant.h"
#include "Win32.h"

using namespace Httpd;
using namespace std;

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

	class DeleteFiberCompletion: public Completion
	{
	public:
		DeleteFiberCompletion()
			: Completion(GetCurrentFiber())
		{}

		virtual bool operator()()
		{
			this->Release();
			// This object is allocated on stack and no longer exist after fiber deletion

			return true;
		}
	};
}

namespace Httpd
{
	class SleepCompletion: public Completion
	{
	public:
		SleepCompletion(int due)
			: Completion(GetCurrentFiber()), due(due)
		{}

		virtual bool operator()()
		{
			Dispatcher &d = Dispatcher::Instance();
			if (!QueueUserAPC(&SetTimerApc, d.hTimerThread, reinterpret_cast<ULONG_PTR>(this))) {
				return false;
			}

			return true;
		}

	private:
		static void CALLBACK SetTimerApc(ULONG_PTR dwParam)
		{
			// N.B. We're executing in timer thread context, and we need not to
			// acquire any lock because APC is conservative.

			Dispatcher &d = Dispatcher::Instance();
			SleepCompletion &sc = *reinterpret_cast<SleepCompletion *>(dwParam);

			// FIXME: This may fail
			d.timerQueue.insert(make_pair(d.GetTickCount64Unsafe() + sc.due, &sc));
		}

	private:
		int due;
	};

	Dispatcher &Dispatcher::Instance()
	{
		static Dispatcher *d(new Dispatcher());
		return *d;
	}

	Dispatcher::Dispatcher()
		: hTimerThread(NULL)
	{
		if ((this->dwTlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES
			|| (this->hQueue = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
			throw FatalException();
	}

	UInt64 Dispatcher::GetTickCount64Unsafe()
	{
		static UInt32 tickHi = 0, tickLoPrevious = 0;
		ULONG tickLo = GetTickCount();
		if (tickLo < tickLoPrevious) {
			++tickHi;
		}
		tickLoPrevious = tickLo;
		return (static_cast<UInt64>(tickHi) << 32) | tickLo;
	}

	void Dispatcher::ThreadEntry()
	{
		if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(),
			&this->hTimerThread, 0, FALSE, DUPLICATE_SAME_ACCESS))
		{
			throw FatalException();
		}

		// Create dispatcher threads
		for (int i = 0; i < DispatcherThreadCount; ++i) {
			HANDLE hThread = CreateThread(NULL, StackReserveSize, &ThreadCallback, this, 0, NULL);
			if (hThread == NULL)
				throw FatalException();
			CloseHandle(hThread);
		}

		while (true) {
			// Fire expired timers
			UInt64 tick = GetTickCount64Unsafe();

			auto iter = timerQueue.begin();
			while (iter != timerQueue.end() && iter->first <= tick) {
				auto cur = iter;
				SleepCompletion &sc = *(cur->second);
				++iter;
				timerQueue.erase(cur);

				// FIXME: This may fail, too
				PostQueuedCompletionStatus(this->hQueue, 0, SleepCompletionKey,
					reinterpret_cast<LPOVERLAPPED>(&sc));
			}

			if (iter == timerQueue.end()) {
				SleepEx(INFINITE, TRUE);
			} else {
				SleepEx(static_cast<DWORD>(iter->first - tick + 50), TRUE);
			}
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
				OverlappedCompletion &oc = *static_cast<OverlappedCompletion *>(lpOverlapped);
				oc.SwitchBack();
			} else if (CompletionKey == SleepCompletionKey) {
				SleepCompletion &sc = *reinterpret_cast<SleepCompletion *>(lpOverlapped);
				sc.SwitchBack();
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
		{
			delete fcr;
			DeleteFiber(lpFiber);
			throw SystemException();
		}
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

		if (threadData.failed) {
			throw SystemException();
		}

		DWORD dwBytesTransferred;
		if (!GetOverlappedResult(hObject, &oc, &dwBytesTransferred, FALSE)) {
			switch (GetLastError()) {
			case ERROR_HANDLE_EOF:
			case ERROR_BROKEN_PIPE:
				return 0;
			default:
				throw SystemException();
			}
		}

		return dwBytesTransferred;
	}

	void Dispatcher::Sleep(int due)
	{
		ThreadData &threadData = *static_cast<ThreadData *>(TlsGetValue(this->dwTlsIndex));
		SleepCompletion sc(due);
		threadData.completion = &sc;
		SwitchToFiber(threadData.lpMainFiber);

		if (threadData.failed) {
			throw SystemException();
		}
	}
}
