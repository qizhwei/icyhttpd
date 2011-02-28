#include "BufferedWriter.h"
#include <cstring>

using namespace std;

namespace Httpd
{
	BufferedWriter::BufferedWriter(Writable &stream, size_t bufferMax)
		: stream(stream), bufferMax(bufferMax)
	{}

	void BufferedWriter::Write(const char *buffer, UInt32 size)
	{
		if (size < this->bufferMax) {
			this->buffer.insert(this->buffer.end(), buffer, buffer + size);
			if (this->buffer.size() >= this->bufferMax)
				this->Flush();
		} else {
			this->Flush();
			this->stream.Write(buffer, size);
		}
	}

	void BufferedWriter::Append(UInt16 i)
	{
		char buffer[8];
		char *p = buffer + 8;
		
		*--p = '\0';
		if (i == 0) {
			*--p = '0';
		} else {
			do {
				*--p = '0' + i % 10;
				i /= 10;
			} while (i != 0);
		}

		this->Write(p, buffer + 8 - p);
	}

	void BufferedWriter::Append(const char *str)
	{
		size_t len = strlen(str);
		this->Write(str, len);
	}

	void BufferedWriter::AppendLine(const char *str)
	{
		this->Append(str);
		this->Write("\r\n", 2);
	}

	void BufferedWriter::AppendLine()
	{
		this->Write("\r\n", 2);
	}

	void BufferedWriter::Flush()
	{
		size_t size;

		if ((size = this->buffer.size()) != 0) {
			this->stream.Write(&*this->buffer.begin(), size);
			this->buffer.clear();
		}
	}
}
