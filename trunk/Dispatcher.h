#ifndef _DISPATCHER_H
#define _DISPATCHER_H

#include "Types.h"
#include "Win32.h"
#include "Exception.h"

namespace Httpd
{
	class Operation
	{
	public:
		Operation(LPVOID lpFiber = GetCurrentFiber())
			: lpFiber(lpFiber)
		{}

		virtual bool operator()() = 0;

		void SwitchBack()
		{
			SwitchToFiber(this->lpFiber);
		}

	private:
		LPVOID lpFiber;

		// This object should not be allocated on heap
		void *operator new(size_t);
		void operator delete(void *);
	};

	class OverlappedOperation: public OVERLAPPED, public Operation
	{
	public:
		OverlappedOperation(UInt64 offset = 0)
		{
			this->Internal = 0;
			this->InternalHigh = 0;

			LARGE_INTEGER liOffset;
			liOffset.QuadPart = offset;
			this->Offset = liOffset.LowPart;
			this->OffsetHigh = liOffset.HighPart;

			this->hEvent = NULL;
		}
	};

	class Dispatcher: NonCopyable
	{
	public:
		static Dispatcher &Instance();
	public:
		void Queue(Callback *callback, void *param);
		void BindHandle(HANDLE hFile, ULONG_PTR key);
		Int32 Block(HANDLE hObject, OverlappedOperation &operation);
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
