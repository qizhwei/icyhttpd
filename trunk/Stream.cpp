#include "Stream.h"
#include "Types.h"
#include "Exception.h"
#include "Constant.h"
#include "Utility.h"
#include <algorithm>
#include <cstring>

namespace Httpd
{
	BufferedReader::BufferedReader(Readable &stream, UInt32 maxBuffer)
		: stream(stream), maxBuffer(maxBuffer), buffer(1, '\0'), current(1), next(UINT32_MAX)
	{}

	char *BufferedReader::ReadLine(bool trimRight)
	{
		UInt32 bufferSize = buffer.size();

		while (next == UINT32_MAX) {
			UInt32 const oldSize = bufferSize;
			if (oldSize == maxBuffer)
				throw SystemException();
			UInt32 const blockSize = std::min(BufferBlockSize, maxBuffer - oldSize);
			UInt32 const newSize = oldSize + blockSize;
			buffer.resize(newSize);

			UInt32 const readSize = stream.Read(&buffer[oldSize], blockSize);
			bufferSize = oldSize + readSize;
			buffer.resize(bufferSize);
			if (readSize == 0)
				return nullptr;

			next = oldSize;
			do {
				if (next == bufferSize) {
					next = UINT32_MAX;
					break;
				}
			} while (buffer[next++] != '\n');
		}

		char *first = &buffer[current];
		char *last = &buffer[next - 1];

		// Eat CRLF
		*last = '\0';
		if (last[-1] == '\r')
			*--last = '\0';

		if (trimRight)
			EatLWSBackwardsUnsafe(last);

		// Maintain pointers
		current = next;
		do {
			if (next == bufferSize) {
				next = UINT32_MAX;
				break;
			}
		} while (buffer[next++] != '\n');

		return first;
	}

	UInt32 BufferedReader::Read(char *buffer, UInt32 size)
	{
		UInt32 const bufferSize = this->buffer.size();
		UInt32 const remain = bufferSize - current;
		if (remain != 0) {
			if (remain < size)
				size = remain;
			memcpy(buffer, &this->buffer[current], size);
			current += size;
			if (current >= next) {
				next = current;
				do {
					if (next == bufferSize) {
						next = UINT32_MAX;
						break;
					}
				} while (this->buffer[next++] != '\n');
			}

			return size;
		} else {
			return this->stream.Read(buffer, size);
		}
	}

	void BufferedReader::Flush()
	{
		UInt32 const shift = current - 1;
		UInt32 const bufferSize = buffer.size();
		UInt32 const shiftSize = bufferSize - current;

		if (shift != 0) {
			if (shiftSize != 0)
				memmove(&buffer[1], &buffer[1 + shift], shiftSize);
			buffer.resize(bufferSize - shift);
			current -= shift;
			if (next != UINT32_MAX)
				next -= shift;
		}
	}

	BufferedWriter::BufferedWriter(Writable &stream)
		: stream(stream)
	{
	}

	void BufferedWriter::Write(const char *buffer, UInt32 size)
	{
		if (size < BufferBlockSize) {
			this->buffer.insert(this->buffer.end(), buffer, buffer + size);
			if (this->buffer.size() >= BufferBlockSize)
				this->Flush();
		} else {
			this->Flush();
			this->stream.Write(buffer, size);
		}
	}

	void BufferedWriter::Append(bool b)
	{
		if (b)
			this->Write("true", 4);
		else
			this->Write("false", 5);
	}

	void BufferedWriter::Append(UInt16 i)
	{
		char buffer[8];
		char *p = ParseUInt<8>(buffer, i);
		this->Write(p, buffer + 8 - p);
	}

	void BufferedWriter::Append(UInt64 i)
	{
		char buffer[24];
		char *p = ParseUInt<24>(buffer, i);
		this->Write(p, buffer + 24 - p);
	}

	void BufferedWriter::Append(const char *str)
	{
		UInt32 len = strlen(str);
		this->Write(str, len);
	}

	void BufferedWriter::AppendLine(bool b)
	{
		this->Append(b);
		this->Write("\r\n", 2);
	}
	
	void BufferedWriter::AppendLine(UInt16 i)
	{
		this->Append(i);
		this->Write("\r\n", 2);
	}	

	void BufferedWriter::AppendLine(UInt64 i)
	{
		this->Append(i);
		this->Write("\r\n", 2);
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
		UInt32 size;

		if ((size = this->buffer.size()) != 0) {
			this->stream.Write(&*this->buffer.begin(), size);
			this->buffer.clear();
		}
	}
}
