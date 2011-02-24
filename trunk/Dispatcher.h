#ifndef _DISPATCHER_H
#define _DISPATCHER_H

#include "Types.h"
#include "Win32.h"

namespace Httpd
{
	struct OverlappedOperation: public OVERLAPPED
	{
		LPVOID lpFiber;
	};

	class Dispatcher: NonCopyable
	{
	public:
		static Dispatcher *Instance();
	public:
		void Queue(Callback *callback, void *param);
		void BindHandle(HANDLE hFile, ULONG_PTR key);
		void Block();
	private:
		Dispatcher();

		// this class has no destructor, declaration only
		~Dispatcher();
		static DWORD WINAPI ThreadCallback(LPVOID);
		static VOID CALLBACK RecyclerCallback(PVOID);
		static VOID CALLBACK FiberCallback(PVOID);
	private:
		HANDLE hQueue;
		DWORD dwMainFiberTls;
		DWORD dwRecycleFiberTls;
		DWORD dwDeletingFiberTls;
	};

}

#endif
