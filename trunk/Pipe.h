#ifndef _PIPE_H
#define _PIPE_H

#include "Types.h"

namespace Httpd
{
	class Pipe: NonCopyable, public Readable, public Writable
	{
	public:
		Pipe();
		virtual ~Pipe();

		virtual UInt32 Read(char *buffer, UInt32 size);
		virtual void Write(const char *buffer, UInt32 size);

	private:
		HANDLE hPipe;
	};
}

#endif
