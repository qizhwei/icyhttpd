#include "File.h"
#include "Dispatcher.h"
#include "Constant.h"
#include "Win32.h"
#include "Exception.h"

namespace Httpd
{
	File::File(const wchar_t *path, bool readOnly)
	{
		if ((this->hFile = CreateFileW(path, readOnly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
			readOnly ? FILE_SHARE_READ : 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE)
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
		overlapped.lpFiber = GetCurrentFiber();

		if (!ReadFile(this->hFile, buffer, size, NULL, &overlapped)
			&& GetLastError() != ERROR_IO_PENDING)
			throw SystemException();

		Dispatcher::Instance()->Block();

		DWORD BytesTransferred;
		if (!GetOverlappedResult(this->hFile, &overlapped, &BytesTransferred, FALSE))
			throw SystemException();
		
		return BytesTransferred;
	}

	void File::Write(char *buffer, UInt32 size)
	{
		throw NotImplementedException();
	}

	UInt64 File::Size()
	{
		LARGE_INTEGER liOffset;
		if (!GetFileSizeEx(this->hFile, &liOffset))
			throw SystemException();
		return liOffset.QuadPart;
	}

	void File::Seek(Int64 offset, int method)
	{
		LARGE_INTEGER liOffset;
		liOffset.QuadPart = offset;
		if (!SetFilePointerEx(this->hFile, liOffset, NULL, method))
			throw SystemException();
	}
}
