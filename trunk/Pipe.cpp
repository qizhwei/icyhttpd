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
		ReadOperation(HANDLE hFile, char *buffer, UInt32 size)
			: hFile(hFile), buffer(buffer), size(size)
		{}

		virtual bool operator()()
		{
			return ReadFile(this->hFile, this->buffer, this->size, NULL, this)
				|| GetLastError() == ERROR_IO_PENDING;
		}

	private:
		HANDLE hFile;
		char *buffer;
		UInt32 size;
	};

	class WriteOperation: public OverlappedOperation
	{
	public:
		WriteOperation(HANDLE hFile, const char *buffer, UInt32 size)
			: hFile(hFile), buffer(buffer), size(size)
		{}

		virtual bool operator()()
		{
			return WriteFile(this->hFile, this->buffer, this->size, NULL, this)
				|| GetLastError() == ERROR_IO_PENDING;
		}

	private:
		HANDLE hFile;
		const char *buffer;
		UInt32 size;
	};
}

namespace Httpd
{
	Pipe::Pipe(HANDLE hPipe)
		: pipe(hPipe)
	{}

	UInt32 Pipe::Read(char *buffer, UInt32 size)
	{
		ReadOperation operation(this->pipe.Handle(), buffer, size);
		return Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->pipe.Handle()), operation);
	}

	void Pipe::Write(const char *buffer, UInt32 size)
	{
		WriteOperation operation(this->pipe.Handle(), buffer, size);
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->pipe.Handle()), operation);
	}
}
