#ifndef _DISPATCHER_H
#define _DISPATCHER_H

#include "Types.h"
#include "Win32.h"
#include <map>

namespace Httpd
{
	class OverlappedCompletion;

	// Completion keys
	const ULONG_PTR FiberCreateKey = 0;
	const ULONG_PTR OverlappedCompletionKey = 1;
	const ULONG_PTR SleepCompletionKey = 2;

	class Dispatcher: NonCopyable
	{
		friend class SleepCompletion;
	public:
		static Dispatcher &Instance();
	public:
		void ThreadEntry();
		void Queue(Callback *callback, void *param);
		void BindHandle(HANDLE hFile, ULONG_PTR key);
		UInt32 Block(HANDLE hObject, OverlappedCompletion &oc);
		void Sleep(int due);

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
		std::multimap<UInt64, SleepCompletion *> timerQueue;
	};
}

#endif
