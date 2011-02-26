#include "File.h"
#include "Dispatcher.h"
#include "Constant.h"
#include "Win32.h"
#include "Exception.h"

namespace Httpd
{
	File::File(const wchar_t *path)
		: offset(0)
	{
		if ((this->hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE)
		{
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
				throw FileNotFoundException();
			else
				throw Exception();
		}

		try {
			Dispatcher::Instance().BindHandle(this->hFile, OverlappedOperationKey);
		} catch (...) {
			CloseHandle(this->hFile);
			throw;
		}
	}

	File::~File()
	{
		CloseHandle(hFile);
	}

	UInt32 File::Read(char *buffer, UInt32 size)
	{
		OverlappedOperation overlapped(this->offset);
		
		if (!ReadFile(this->hFile, buffer, size, NULL, &overlapped)
			&& GetLastError() != ERROR_IO_PENDING)
			throw Exception();

		return Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hFile), overlapped);
	}

	UInt64 File::Size()
	{
		LARGE_INTEGER liFileSize;
		if (!GetFileSizeEx(this->hFile, &liFileSize))
			throw Exception();
		return liFileSize.QuadPart;
	}

	void File::Seek(UInt64 offset)
	{
		this->offset = offset;
	}
}
