#include "Win32.h"
#include "Pipe.h"
#include "Dispatcher.h"
#include "Exception.h"
#include "Constant.h"

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

	class WriteOperation: public OverlappedOperation
	{
	public:
		WriteOperation(HANDLE hFile, Int64 offset, const char *buffer, UInt32 size)
			: hFile(hFile), OverlappedOperation(offset), buffer(buffer), size(size)
		{}

		virtual bool operator()()
		{
			return static_cast<bool>(WriteFile(this->hFile, this->buffer, this->size,
				NULL, this) || GetLastError() == ERROR_IO_PENDING);
		}

	private:
		HANDLE hFile;
		const char *buffer;
		UInt32 size;
	};
}

namespace Httpd
{
	static long PipeCount = 0; 
	PipeHandle::PipeHandle(HANDLE hPipe)
		: hPipe(hPipe)
	{
		const int BUFFER_SIZE = 1024; 
		const int PIPE_TIMEOUT = 1000; 
		TCHAR PIPE_NAME[48];
		
		wsprintf(PIPE_NAME, L"\\\\.\\pipe\\icyhttpd\\critter.%08x.%08x", GetCurrentProcessId(),  InterlockedIncrement(&PipeCount));
		
		SECURITY_ATTRIBUTES sec; 
		sec.bInheritHandle = TRUE;
		sec.lpSecurityDescriptor = NULL;
		sec.nLength = sizeof(SECURITY_ATTRIBUTES);

		if ((hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX 
                , PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, BUFFER_SIZE, BUFFER_SIZE 
                , PIPE_TIMEOUT, &sec)) == INVALID_HANDLE_VALUE ){
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
				throw NotFoundException();
			else
				throw SystemException();
		}

		try {
			Dispatcher::Instance().BindHandle(hPipe, OverlappedOperationKey);
		} catch (...) {
			CloseHandle(hPipe);
			throw;
		}
	}

	HANDLE PipeHandle::Handle()
	{
		return hPipe;
	}

	PipeHandle::~PipeHandle()
	{
		CloseHandle(hPipe);
	}

	UInt32 PipeReader::Read(char *buffer, UInt32 size)
	{
		Int32 result;

		ReadOperation operation(this->pipe.Handle(), 0, buffer, size);
		
		result = Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->pipe.Handle()), operation);
		if (result == -1)
			throw SystemException();

		return static_cast<UInt32>(result);
	}

	void PipeWriter::Write(const char *buffer, UInt32 size)
	{
		Int32 result;

		WriteOperation operation(this->pipe.Handle(), 0, buffer, size);
		
		result = Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->pipe.Handle()), operation);
		if (result == -1)
			throw SystemException();
	}

	UInt32 Pipe::Read(char *buffer, UInt32 size)
	{
		Int32 result;

		ReadOperation operation(this->pipe.Handle(), 0, buffer, size);
		
		result = Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->pipe.Handle()), operation);
		if (result == -1)
			throw SystemException();

		return static_cast<UInt32>(result);
	}

	void Pipe::Write(const char *buffer, UInt32 size)
	{
		Int32 result;

		WriteOperation operation(this->pipe.Handle(), 0, buffer, size);
		
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->pipe.Handle()), operation);
		if (result == -1)
			throw SystemException();
	
	}
}
