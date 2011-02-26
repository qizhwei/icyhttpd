#ifndef _FILE_H
#define _FILE_H

#include "Types.h"
#include "Win32.h"
#include "Constant.h"

namespace Httpd
{
	class File: NonCopyable, public Readable
	{
	public:
		File(const wchar_t *path);
		virtual ~File();
		virtual UInt32 Read(char *buffer, UInt32 size);
		UInt64 Size();
		void Seek(UInt64 offset);
	private:
		HANDLE hFile;
		UInt64 offset;
	};
}

#endif
