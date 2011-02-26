#ifndef _DISPATCHER_H
#define _DISPATCHER_H

#include "Types.h"
#include "Win32.h"
#include "Exception.h"

namespace Httpd
{
	struct OverlappedOperation: public OVERLAPPED
	{
		OverlappedOperation(UInt64 offset = 0, LPVOID lpFiber = GetCurrentFiber())
		{
			this->Internal = 0;
			this->InternalHigh = 0;

			LARGE_INTEGER liOffset;
			liOffset.QuadPart = offset;
			this->Offset = liOffset.LowPart;
			this->OffsetHigh = liOffset.HighPart;

			this->hEvent = NULL;
			this->lpFiber = lpFiber;

			if ((this->hReadyEvent = CreateEventW(NULL, FALSE, FALSE, NULL)) == NULL)
				throw FatalException();
		}

		LPVOID lpFiber;
		HANDLE hReadyEvent;
	};

	class Dispatcher: NonCopyable
	{
	public:
		static Dispatcher &Instance();
	public:
		void Queue(Callback *callback, void *param);
		void BindHandle(HANDLE hFile, ULONG_PTR key);
		UInt32 Block(HANDLE hObject, OverlappedOperation &overlapped);
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
