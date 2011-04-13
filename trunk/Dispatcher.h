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
		friend class WakeCompletion;
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
	const ULONG_PTR WakeCompletionKey = 3;

	class Dispatcher: NonCopyable
	{
		friend class SleepCompletion;
		friend class WakeCompletion;
		friend class WakeToken;
	public:
		static Dispatcher &Instance();
	public:
		void ThreadEntry();
		void Queue(Callback *callback, void *param);
		void BindHandle(HANDLE hFile, ULONG_PTR key);
		UInt32 Block(HANDLE hObject, OverlappedCompletion &oc);
		bool Sleep(int due, std::shared_ptr<WakeToken> ct = nullptr);

	private:
		Dispatcher();

		// this class has no destructor, declaration only
		~Dispatcher();
		void Post(DWORD dwNumberOfBytesTransferred,
			ULONG_PTR dwCompletionKey, LPOVERLAPPED lpOverlapped);

		static DWORD WINAPI ThreadCallback(LPVOID);
		static VOID CALLBACK FiberCallback(PVOID);
		static UInt64 GetTickCount64Unsafe();
	private:
		HANDLE hQueue;
		DWORD dwTlsIndex;
		HANDLE hTimerThread;
		TimerQueue timerQueue;
	};
}

#endif
