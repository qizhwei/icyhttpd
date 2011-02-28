#include "Win32.h"
#include "Pipe.h"
#include "Dispatcher.h"
#include "Exception.h"
#include "Constant.h"

using namespace Httpd;

namespace
{
	volatile long PipeCount = 0;
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
	PipeHandle::PipeHandle(HANDLE hPipe)
		: hPipe(hPipe)
	{
		const int BUFFER_SIZE = 1024; 
		const int PIPE_TIMEOUT = 1000; 
		wchar_t PipeName[48];
		
		wsprintf(PipeName, L"\\\\.\\pipe\\icyhttpd\\critter.%08x.%08x", GetCurrentProcessId(),  InterlockedIncrement(&PipeCount));

		hPipe = CreateNamedPipe(PipeName, PIPE_ACCESS_DUPLEX 
				, PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, BUFFER_SIZE, BUFFER_SIZE 
				, PIPE_TIMEOUT,NULL);

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

	PipeReader::PipeReader(HANDLE hPipe)
		: pipe(hPipe)
	{}

	UInt32 PipeReader::Read(char *buffer, UInt32 size)
	{
		ReadOperation operation(this->pipe.Handle(), 0, buffer, size);
		return Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->pipe.Handle()), operation);
	}

	PipeWriter::PipeWriter(HANDLE hPipe)
		: pipe(hPipe)
	{}

	void PipeWriter::Write(const char *buffer, UInt32 size)
	{
		WriteOperation operation(this->pipe.Handle(), 0, buffer, size);
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->pipe.Handle()), operation);
	}

	Pipe::Pipe(HANDLE hPipe)
		: pipe(hPipe)
	{}

	UInt32 Pipe::Read(char *buffer, UInt32 size)
	{
		ReadOperation operation(this->pipe.Handle(), 0, buffer, size);
		return Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->pipe.Handle()), operation);
	}

	void Pipe::Write(const char *buffer, UInt32 size)
	{
		WriteOperation operation(this->pipe.Handle(), 0, buffer, size);
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->pipe.Handle()), operation);
	}
}
