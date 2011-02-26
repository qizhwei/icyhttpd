#ifndef _RESPONSE_H
#define _RESPONSE_H

#include "Types.h"

namespace Httpd
{
	class Response: NonCopyable, public Writable
	{
	public:
		Response(Writable &stream, UInt16 status);

		// TODO: Write header and End header

		virtual void Write(const char *buffer, UInt32 size);
	private:
		Writable &stream;
	};
}

#endif
