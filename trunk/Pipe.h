#ifndef _PIPE_H
#define _PIPE_H

#include "Types.h"
#include "Win32.h"
#include "Utility.h"

namespace Httpd
{
	class Pipe
	{
	public:
		Pipe(HANDLE hPipe);
		UInt32 Read(char *buffer, UInt32 size);
		void Write(const char *buffer, UInt32 size);
	private:
		Win32Handle pipe;
	};

	class LocalPipe
	{
	public:
		LocalPipe(std::pair<HANDLE, HANDLE> hPipes);
		UInt32 Read(char *buffer, UInt32 size);
		void Write(const char *buffer, UInt32 size);
		void CloseRead();
		void CloseWrite();
	private:
		Win32Handle readPipe, writePipe;
	};
}

#endif
