#ifndef _FILE_H
#define _FILE_H

#include "Types.h"
#include "Win32.h"
#include "Stream.h"
#include "Constant.h"

namespace Httpd
{
	class File: NonCopyable, public Stream
	{
	public:
		File(const wchar_t *path, bool readOnly = true);
		virtual ~File();
		virtual UInt32 Read(char *buffer, UInt32 size);
		virtual void Write(char *buffer, UInt32 size);
		UInt64 Size();
		void Seek(Int64 offset, int method = SeekBegin);
	private:
		HANDLE hFile;
	};
}

#endif
