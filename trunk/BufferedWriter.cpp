#include "BufferedWriter.h"

namespace Httpd
{
	BufferedWriter::BufferedWriter(Writable &stream, size_t bufferSize)
		: stream(stream), buffer(bufferSize)
	{}

	void BufferedWriter::Write(const char *buffer, UInt32 size)
	{
	}

	void BufferedWriter::Append(UInt16 i)
	{
	}

	void BufferedWriter::Append(const char *str)
	{
	}

	void BufferedWriter::AppendLine(const char *str)
	{
	}

	void BufferedWriter::Flush()
	{
	}
}
