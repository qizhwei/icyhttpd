#ifndef _FCGIPROCESS_H
#define _FCGIPROCESS_H

#include "Types.h"
#include "Win32.h"
#include "Utility.h"
#include <string>

namespace Httpd
{
	class FcgiProcess: NonCopyable, public Shared<FcgiProcess>
	{
	public:
		FcgiProcess(const std::wstring &commandLine, UInt32 maxRequests);
		~FcgiProcess();
		static void ReadPipeCallback(void *param);
		// TODO:
		// Layer 0: Read / Write packets
		// Layer 1: Sessions and streams

	private:
		HANDLE hProcess;
		HANDLE hPipe;
	};
}

#endif
