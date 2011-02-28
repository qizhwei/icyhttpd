#include "File.h"
#include "Dispatcher.h"
#include "Constant.h"
#include "Win32.h"
#include "Exception.h"

using namespace Httpd;

namespace
{
	class ReadOperation: public OverlappedOperation
	{
	public:
		ReadOperation(HANDLE hFile, Int64 offset, char *buffer, UInt32 size)
			: hFile(hFile), OverlappedOperation(offset), buffer(buffer), size(size)
		{}

		virtual bool operator()()
		{
			return static_cast<bool>(ReadFile(this->hFile, this->buffer, this->size,
				NULL, this) || GetLastError() == ERROR_IO_PENDING);
		}

	private:
		HANDLE hFile;
		char *buffer;
		UInt32 size;
	};
}

namespace Httpd
{
	File::File(const wchar_t *path)
		: offset(0)
	{
		if ((this->hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE)
		{
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
				throw NotFoundException();
			else
				throw SystemException();
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
		ReadOperation operation(this->hFile, this->offset, buffer, size);
		UInt32 result = Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hFile), operation);
		this->offset += result;
		return result;
	}

	UInt64 File::Size()
	{
		LARGE_INTEGER liFileSize;
		if (!GetFileSizeEx(this->hFile, &liFileSize))
			throw SystemException();
		return liFileSize.QuadPart;
	}

	void File::Seek(UInt64 offset)
	{
		this->offset = offset;
	}
}
