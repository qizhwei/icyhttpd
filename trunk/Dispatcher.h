#ifndef _DISPATCHER_H
#define _DISPATCHER_H

#include "Types.h"
#include "Win32.h"

namespace Httpd
{
	class OverlappedCompletion;

	// Completion keys
	const ULONG_PTR FiberCreateKey = 0;
	const ULONG_PTR OverlappedCompletionKey = 1;

	class Dispatcher: NonCopyable
	{
	public:
		static Dispatcher &Instance();
	public:
		void Queue(Callback *callback, void *param);
		void BindHandle(HANDLE hFile, ULONG_PTR key);
		UInt32 Block(HANDLE hObject, OverlappedCompletion &oc);
	private:
		Dispatcher();

		// this class has no destructor, declaration only
		~Dispatcher();
		static DWORD WINAPI ThreadCallback(LPVOID);
		static VOID CALLBACK FiberCallback(PVOID);
	private:
		HANDLE hQueue;
		DWORD dwTlsIndex;
	};
}

#endif
