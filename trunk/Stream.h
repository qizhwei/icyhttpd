#ifndef _STREAM_H
#define _STREAM_H

#include "Types.h"

namespace Httpd
{
	class Stream
	{
	public:
		virtual UInt32 Read(char *buffer, UInt32 size) = 0;
		virtual void Write(char *buffer, UInt32 size) = 0;
		virtual ~Stream() {};
	};
}

#endif
