#ifndef _FCGIPOOL_H
#define _FCGIPOOL_H

#include "Types.h"
#include <string>

namespace Httpd
{
	class FcgiProcess;

	class FcgiPool: NonCopyable
	{
	public:
		FcgiPool(const std::wstring &commandLine, UInt32 idleTime, UInt32 maxRequests);
		void Push(FcgiProcess *process);
		FcgiProcess *Pop();

	private:
		~FcgiPool();
		
	private:
		std::wstring commandLine;
		UInt32 idleTime;
		UInt32 maxRequests;
	};
}

#endif
