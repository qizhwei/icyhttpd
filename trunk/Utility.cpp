#include "Utility.h"
#include "Types.h"
#include "Exception.h"
#include "Constant.h"

using namespace Httpd;

namespace
{
	volatile long PipeCount;
	const wchar_t PipeNameFormat[] = L"\\\\.\\pipe\\icyhttpd\\critter.%08x.%08x";
}

namespace Httpd
{
	void CreatePipePairDuplex(HANDLE hPipe[2])
	{
		wchar_t PipeName[48];
		HANDLE hPipe0, hPipe1;
		
		wsprintfW(PipeName, PipeNameFormat, GetCurrentProcessId(), InterlockedIncrement(&PipeCount));

		if ((hPipe0 = CreateNamedPipeW(PipeName, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_READMODE_BYTE, 1,
			PipeBufferSize, PipeBufferSize, 0, NULL)) == INVALID_HANDLE_VALUE)
			throw SystemException();

		if ((hPipe1 = CreateFileW(PipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hPipe0);
			throw SystemException();
		}

		hPipe[0] = hPipe0;
		hPipe[1] = hPipe1;
	}
	
	void CreatePipePair(HANDLE hPipe[2])
	{
		wchar_t PipeName[48];
		HANDLE hPipe0, hPipe1;
		
		wsprintfW(PipeName, PipeNameFormat, GetCurrentProcessId(), InterlockedIncrement(&PipeCount));

		if ((hPipe0 = CreateNamedPipe(PipeName, PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_READMODE_BYTE, 1,
			PipeBufferSize, PipeBufferSize, 0, NULL)) == INVALID_HANDLE_VALUE)
			throw SystemException();

		if ((hPipe1 = CreateFile(PipeName, GENERIC_WRITE, 0, NULL,
			OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hPipe0);
			throw SystemException();
		}

		hPipe[0] = hPipe0;
		hPipe[1] = hPipe1;
	}

	HANDLE OpenFile(const wchar_t *path)
	{
		HANDLE hFile;

		if ((hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE)
		{
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
				throw NotFoundException();
			else
				throw SystemException();
		}

		return hFile;
	}

	UInt64 GetFileSize(HANDLE hFile)
	{
		LARGE_INTEGER liFileSize;
		if (!GetFileSizeEx(hFile, &liFileSize))
			throw SystemException();
		return liFileSize.QuadPart;
	}
}