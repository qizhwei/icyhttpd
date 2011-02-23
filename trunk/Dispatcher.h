#ifndef _DISPATCHER_H
#define _DISPATCHER_H

#include "Types.h"
#include "Win32.h"

namespace Httpd
{
	class Dispatcher: NonCopyable
	{
	public:
		static Dispatcher *Instance();
	public:
		void Queue(Callback *, void *);
	private:
		Dispatcher();

		// this class has no destructor, declaration only
		~Dispatcher();
		void SwitchBack();
		static DWORD WINAPI ThreadCallback(LPVOID);
		static VOID CALLBACK FiberCallback(PVOID);
	private:
		HANDLE hQueue;
		DWORD dwMainFiberTls;
	};

}

#endif
