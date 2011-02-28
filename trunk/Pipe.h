#ifndef _PIPE_H
#define _PIPE_H

#include "Types.h"
#include "Win32.h"

namespace Httpd
{
	class PipeHandle: NonCopyable
	{
	public:
		HANDLE Handle();
		PipeHandle(HANDLE hPipe);
		~PipeHandle();
	private:
		HANDLE hPipe;
	};

	class PipeReader: public Readable
	{
	public:
		PipeReader(HANDLE hPipe);
		virtual UInt32 Read(char *buffer, UInt32 size);
	private:
		PipeHandle pipe;
	};

	class PipeWriter: public Writable
	{
	public:
		PipeWriter(HANDLE hPipe);
		virtual void Write(const char *buffer, UInt32 size);
	private:
		PipeHandle pipe;
	};

	class Pipe: public Readable, public Writable
	{
	public:
		Pipe(HANDLE hPipe);
		virtual UInt32 Read(char *buffer, UInt32 size);
		virtual void Write(const char *buffer, UInt32 size);
	private:
		PipeHandle pipe;
	};
}

#endif
