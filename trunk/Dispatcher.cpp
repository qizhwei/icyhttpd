#include "Dispatcher.h"
#include "Utility.h"
#include "Exception.h"
#include "Types.h"
#include "Constant.h"
#include "Win32.h"
#include <stdexcept>
#include <memory>

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
		friend class Dispatcher;
		friend class WakeToken;
	public:
		SleepCompletion(int due, WakeToken *wt)
			: Completion(GetCurrentFiber()), due(due), wt(wt)
		{}

		virtual bool operator()()
		{
			if (!QueueUserAPC(&SetTimerApc, Dispatcher::Instance().hTimerThread,
				reinterpret_cast<ULONG_PTR>(this)))
			{
				return false;
			}
			return true;
		}

	private:
		static VOID CALLBACK SetTimerApc(ULONG_PTR dwParam)
		{
			// N.B. We're executing in timer thread context, and we need not to
			// acquire any lock because APC is conservative.

			Dispatcher &d = Dispatcher::Instance();
			SleepCompletion &sc = *reinterpret_cast<SleepCompletion *>(dwParam);
			TimerQueue::iterator iterator;

			try {
				iterator = d.timerQueue.insert(make_pair(d.GetTickCount64Unsafe() + sc.due, &sc));
			} catch (const exception &) {
				d.Post(1, GeneralCompletionKey, reinterpret_cast<LPOVERLAPPED>(
					static_cast<Completion *>(&sc)));
				iterator = d.timerQueue.end();
			}

			if (sc.wt != nullptr) {
				sc.wt->iterator = iterator;
			}
		}

	private:
		union {
			int due;
			bool awaken;
		};
		WakeToken *wt;
	};

	class InvokeApcCompletion: public Completion
	{
		friend class Dispatcher;
	public:
		InvokeApcCompletion(BinaryCallback *callback, void *param)
			: Completion(GetCurrentFiber()), callback(callback), param(param)
		{}

		virtual bool operator()()
		{
			if (!::QueueUserAPC(&InvokeApcApc, Dispatcher::Instance().hTimerThread,
				reinterpret_cast<ULONG_PTR>(this)))
			{
				return false;
			}
			return true;
		}

		void Complete(void *result)
		{
			this->result = result;
			Dispatcher::Instance().Post(0, GeneralCompletionKey,
				reinterpret_cast<LPOVERLAPPED>(static_cast<Completion *>(this)));
		}

	private:
		static VOID CALLBACK InvokeApcApc(ULONG_PTR dwParam)
		{
			InvokeApcCompletion &iac = *reinterpret_cast<InvokeApcCompletion *>(dwParam);
			(*iac.callback)(iac.param, reinterpret_cast<void *>(&iac));
		}

	private:
		BinaryCallback *callback;
		union {
			void *param;
			void *result;
		};
	};

	WakeToken::WakeToken()
		// FIXME: Is this thread safe?
		: iterator(Dispatcher::Instance().timerQueue.end())
	{}

	bool WakeToken::Wake()
	{
		Dispatcher &d = Dispatcher::Instance();
		void *result = d.InvokeApc([this, &d](void *completion)->void
		{
			if (this->iterator == d.timerQueue.end()) {
				d.CompleteApc(completion, reinterpret_cast<void *>(1));
			} else {
				SleepCompletion &sc = *(this->iterator->second);
				d.timerQueue.erase(this->iterator);
				this->iterator = d.timerQueue.end();
				sc.awaken = true;
				d.Post(0, GeneralCompletionKey, reinterpret_cast<LPOVERLAPPED>(
					static_cast<Completion *>(&sc)));
				d.CompleteApc(completion, nullptr);
			}
		});

		return (result == nullptr);
	}

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

	void Dispatcher::Post(DWORD dwNumberOfBytesTransferred,
		ULONG_PTR dwCompletionKey, LPOVERLAPPED lpOverlapped)
	{
		if (!PostQueuedCompletionStatus(this->hQueue, dwNumberOfBytesTransferred,
			dwCompletionKey, lpOverlapped))
		{
			throw FatalException();
		}
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
				if (sc.wt != nullptr) {
					sc.wt->iterator = timerQueue.end();
				}
				sc.awaken = false;
				this->Post(0, GeneralCompletionKey, reinterpret_cast<LPOVERLAPPED>(&sc));
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
				static_cast<OverlappedCompletion *>(lpOverlapped)->SwitchBack();
			} else if (CompletionKey == GeneralCompletionKey) {
				if (NumberOfBytes != 0)
					threadData.failed = true;
				reinterpret_cast<Completion *>(lpOverlapped)->SwitchBack();
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
		this->Post(0, FiberCreateKey, reinterpret_cast<LPOVERLAPPED>(lpFiber));
	}

	void Dispatcher::BindHandle(HANDLE hFile, ULONG_PTR key)
	{
		if (CreateIoCompletionPort(hFile, this->hQueue, key, 0) == NULL)
			throw SystemException();
	}

	UInt32 Dispatcher::Block(HANDLE hObject, OverlappedCompletion &oc)
	{
		ThreadData *threadData = static_cast<ThreadData *>(TlsGetValue(this->dwTlsIndex));
		threadData->completion = &oc;
		SwitchToFiber(threadData->lpMainFiber);

		// N.B. Thread may changed
		threadData = static_cast<ThreadData *>(TlsGetValue(this->dwTlsIndex));
		if (threadData->failed) {
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

	bool Dispatcher::SleepInternal(int due, WakeToken *wt)
	{
		ThreadData *threadData = static_cast<ThreadData *>(TlsGetValue(this->dwTlsIndex));
		SleepCompletion sc(due, wt);
		threadData->completion = &sc;
		SwitchToFiber(threadData->lpMainFiber);

		// N.B. Thread may changed
		threadData = static_cast<ThreadData *>(TlsGetValue(this->dwTlsIndex));
		if (threadData->failed) {
			throw SystemException();
		}

		return sc.awaken;
	}

	void Dispatcher::Sleep(int due)
	{
		this->SleepInternal(due, nullptr);
	}

	bool Dispatcher::Sleep(int due, WakeToken &wt)
	{
		return this->SleepInternal(due, &wt);
	}

	void *Dispatcher::InvokeApc(BinaryCallback *callback, void *param)
	{
		ThreadData *threadData = static_cast<ThreadData *>(TlsGetValue(this->dwTlsIndex));
		InvokeApcCompletion iac(callback, param);
		threadData->completion = &iac;
		SwitchToFiber(threadData->lpMainFiber);

		// N.B. Thread may changed
		threadData = static_cast<ThreadData *>(TlsGetValue(this->dwTlsIndex));
		if (threadData->failed) {
			throw SystemException();
		}

		return iac.result;
	}

	void Dispatcher::CompleteApc(void *completion, void *result)
	{
		InvokeApcCompletion &iac = *reinterpret_cast<InvokeApcCompletion *>(completion);
		iac.Complete(result);
	}
}
