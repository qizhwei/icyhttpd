#ifndef _BUFFEREDWRITER_H
#define _BUFFEREDWRITER_H

#include "Types.h"
#include <cstddef>
#include <vector>

namespace Httpd
{
	class BufferedWriter: NonCopyable, public Writable
	{
	public:
		BufferedWriter(Writable &stream, size_t bufferMax = 4096);
		virtual void Write(const char *buffer, UInt32 size);
		void Append(UInt16 i);
		void Append(const char *str);
		void AppendLine(const char *str);
		void AppendLine();
		void Flush();

	private:
		Writable &stream;
		std::vector<char> buffer;
		size_t bufferMax;
	};
}

#endif
