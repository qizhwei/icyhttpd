#ifndef _FILE_H
#define _FILE_H

#include "Types.h"
#include "Win32.h"
#include "Constant.h"

namespace Httpd
{
	// TODO: Change name to FileReader
	class File: NonCopyable, public Readable
	{
	public:
		// TODO: Accept handle only, move handle creation code to Utility
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
