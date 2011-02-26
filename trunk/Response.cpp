#include "Response.h"
#include "Types.h"

namespace Httpd
{
	Response::Response(Writable &stream, UInt16 status)
		: stream(stream)
	{
		// TODO: Write onto the stream, we need buffered writer
	}

	void Response::Write(const char *buffer, UInt32 size)
	{
		// TODO: Write onto the buffered writer (after finishing the headers)
	}
}
