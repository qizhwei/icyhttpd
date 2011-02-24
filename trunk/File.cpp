#include "File.h"
#include "Dispatcher.h"
#include "Constant.h"
#include "Win32.h"
#include "Exception.h"

namespace Httpd
{
	File::File(const wchar_t *path)
	{
		if ((this->hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE)
		{
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
				throw FileNotFoundException();
			else
				throw SystemException();
		}

		try {
			Dispatcher::Instance()->BindHandle(this->hFile, OverlappedOperationKey);
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
		OverlappedOperation overlapped;
		memset(static_cast<OVERLAPPED *>(&overlapped), 0, sizeof(OVERLAPPED));
		
		overlapped.Offset = this->offset.LowPart;
		overlapped.OffsetHigh = this->offset.HighPart;
		overlapped.lpFiber = GetCurrentFiber();

		if (!ReadFile(this->hFile, buffer, size, NULL, &overlapped)
			&& GetLastError() != ERROR_IO_PENDING)
			throw SystemException();

		Dispatcher::Instance()->Block();

		DWORD dwBytesTransferred;
		if (!GetOverlappedResult(this->hFile, &overlapped, &dwBytesTransferred, FALSE)) {
			if (GetLastError() == ERROR_HANDLE_EOF)
				return 0;
			else
				throw SystemException();
		}

		this->offset.QuadPart += dwBytesTransferred;
		return dwBytesTransferred;
	}

	UInt64 File::Size()
	{
		LARGE_INTEGER liOffset;
		if (!GetFileSizeEx(this->hFile, &liOffset))
			throw SystemException();
		return liOffset.QuadPart;
	}

	void File::Seek(UInt64 offset)
	{
		this->offset.QuadPart = offset;
	}
}
