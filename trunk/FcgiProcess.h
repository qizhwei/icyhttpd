#ifndef _FCGIPROCESS_H
#define _FCGIPROCESS_H

#include "Types.h"
#include "Win32.h"
#include "Utility.h"
#include "Pipe.h"
#include <string>

namespace Httpd
{
	class FcgiProcess: NonCopyable
	{
	public:
		static FcgiProcess *Create(const std::wstring &commandLine, UInt16 maxRequests);
		~FcgiProcess();

		// Acquires a request id, it's equal to the remaining requests available.
		// If the result is zero, the process should be closed after the request.
		UInt16 Acquire();

		UInt32 Read(char *buffer, UInt32 size);
		void Write(const char *buffer, UInt32 size);

	private:
		FcgiProcess(HANDLE hProcess, HANDLE hPipe, LONG maxRequests);

	private:
		Win32Handle process;
		Pipe pipe;
		volatile LONG remainingRequests;
	};
}

#endif
