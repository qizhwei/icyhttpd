#ifndef _FCGIPOOL_H
#define _FCGIPOOL_H

#include "Types.h"
#include "Win32.h"
#include <string>

namespace Httpd
{
	class FcgiPool: NonCopyable
	{
	public:
		FcgiPool(const std::wstring &commandLine, UInt32 idleTime, UInt32 maxRequests);

	private:
		// this class has no destructor, declaration only
		~FcgiPool();
		static DWORD WINAPI ThreadCallback(LPVOID);
		
	private:
		std::wstring commandLine;
		UInt32 idleTime;
		UInt32 maxRequests;
		HANDLE hQueue;
		HANDLE hJob;
	};
}

#endif
