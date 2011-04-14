#ifndef _DISPATCHER_H
#define _DISPATCHER_H

#include "Types.h"
#include "Win32.h"
#include <map>
#include <memory>

namespace Httpd
{
	class OverlappedCompletion;
	class SleepCompletion;
	typedef std::multimap<UInt64, SleepCompletion *> TimerQueue;

	class WakeToken: NonCopyable
	{
		friend class Dispatcher;
		friend class SleepCompletion;
	public:
		WakeToken();
		bool Wake();
	private:
		// N.B. This iterator should only be accessed in timer thread
		TimerQueue::iterator iterator;
	};

	// Completion keys
	const ULONG_PTR FiberCreateKey = 0;
	const ULONG_PTR OverlappedCompletionKey = 1;
	const ULONG_PTR SleepCompletionKey = 2;
	const ULONG_PTR InvokeApcCompletionKey = 3;

	class Dispatcher: NonCopyable
	{
		friend class WakeToken;
		friend class SleepCompletion;
		friend class InvokeApcCompletion;
	public:
		static Dispatcher &Instance();
	public:
		void ThreadEntry();
		void Queue(Callback *callback, void *param);
		void BindHandle(HANDLE hFile, ULONG_PTR key);
		UInt32 Block(HANDLE hObject, OverlappedCompletion &oc);
		void *InvokeApc(InvokeCallback *callback, void *param);
		void Sleep(int due);
		bool Sleep(int due, WakeToken &wt);

		template<typename Lambda>
		void *InvokeApc(Lambda &l)
		{
			return InvokeApc(&LambdaInvokeCallback<Lambda>, reinterpret_cast<void *>(&l)); 
		}

	private:
		Dispatcher();

		// this class has no destructor, declaration only
		~Dispatcher();
		void Post(DWORD dwNumberOfBytesTransferred,
			ULONG_PTR dwCompletionKey, LPOVERLAPPED lpOverlapped);

		static DWORD WINAPI ThreadCallback(LPVOID);
		static VOID CALLBACK FiberCallback(PVOID);
		static UInt64 GetTickCount64Unsafe();

		template<typename Lambda>
		static void *LambdaInvokeCallback(void *param)
		{
			Lambda &l = *reinterpret_cast<Lambda *>(param);
			return l();
		}

		bool SleepInternal(int due, WakeToken *wt);
	private:
		HANDLE hQueue;
		DWORD dwTlsIndex;
		HANDLE hTimerThread;
		TimerQueue timerQueue;
	};
}

#endif
