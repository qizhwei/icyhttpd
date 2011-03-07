#include "Pipe.h"
#include "Dispatcher.h"
#include "Utility.h"
#include "Win32.h"

using namespace Httpd;

namespace
{
	class ReadCompletion: public OverlappedCompletion
	{
	public:
		ReadCompletion(HANDLE hFile, char *buffer, UInt32 size)
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

	class WriteCompletion: public OverlappedCompletion
	{
	public:
		WriteCompletion(HANDLE hFile, const char *buffer, UInt32 size)
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
		ReadCompletion completion(this->pipe.Handle(), buffer, size);
		return Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->pipe.Handle()), completion);
	}

	void Pipe::Write(const char *buffer, UInt32 size)
	{
		WriteCompletion completion(this->pipe.Handle(), buffer, size);
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->pipe.Handle()), completion);
	}
}
