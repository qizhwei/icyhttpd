#ifndef _BUFFEREDWRITER_H
#define _BUFFEREDWRITER_H

#include "Types.h"
#include <vector>

namespace Httpd
{
	class BufferedWriter: NonCopyable, public Writable
	{
	public:
		BufferedWriter(Writable &stream);
		virtual void Write(const char *buffer, UInt32 size);
		void Append(UInt16 i);
		void Append(const char *str);
		void AppendLine(const char *str);
		void Flush();

	private:
		Writable &stream;
		std::vector<char> buffer;
	};
}

#endif
