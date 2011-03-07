#ifndef _STREAM_H
#define _STREAM_H

#include "Types.h"
#include <vector>

namespace Httpd
{
	class Readable
	{
	public:
		virtual UInt32 Read(char *buffer, UInt32 size) = 0;
	};

	class Writable
	{
	public:
		virtual void Write(const char *buffer, UInt32 size) = 0;
	};

	template<typename BaseStream>
	class Reader: NonCopyable, public Readable
	{
	public:
		Reader(BaseStream &stream)
			: stream(stream)
		{}

		virtual UInt32 Read(char *buffer, UInt32 size)
		{
			return stream.Read(buffer, size);
		}
	private:
		BaseStream &stream;
	};

	template<typename BaseStream>
	class Writer: NonCopyable, public Writable
	{
	public:
		Writer(BaseStream &stream)
			: stream(stream)
		{}

		virtual void Write(const char *buffer, UInt32 size)
		{
			stream.Write(buffer, size);
		}
	private:
		BaseStream &stream;
	};

	class BufferedReader: NonCopyable
	{
	public:
		BufferedReader(Readable &stream, UInt32 maxBuffer);
		char *ReadLine(bool trimRight = false);
		char *BasePointer() { return &*buffer.begin(); }
		UInt32 Read(char *buffer, UInt32 size);
		void Flush();

	private:
		Readable &stream;
		UInt32 maxBuffer;
		std::vector<char> buffer;
		UInt32 current, next;
	};

	class BufferedWriter: NonCopyable
	{
	public:
		BufferedWriter(Writable &stream);
		void Write(const char *buffer, UInt32 size);
		void Append(bool b);
		void Append(UInt16 i);
		void Append(UInt64 i);
		void Append(const char *str);
		void AppendLine(bool b);
		void AppendLine(UInt16 i);
		void AppendLine(UInt64 i);
		void AppendLine(const char *str);
		void AppendLine();
		void Flush();

	private:
		Writable &stream;
		UInt32 blockSize;
		std::vector<char> buffer;
	};
}

#endif