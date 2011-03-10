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
	{
		Dispatcher::Instance().BindHandle(hPipe, OverlappedCompletionKey);
	}

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

	LocalPipe::LocalPipe(std::pair<HANDLE, HANDLE> hPipes)
		: readPipe(hPipes.first), writePipe(hPipes.second)
	{
		Dispatcher::Instance().BindHandle(hPipes.first, OverlappedCompletionKey);
		Dispatcher::Instance().BindHandle(hPipes.second, OverlappedCompletionKey);
	}


	UInt32 LocalPipe::Read(char *buffer, UInt32 size)
	{
		ReadCompletion completion(this->readPipe.Handle(), buffer, size);
		return Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->readPipe.Handle()), completion);
	}

	void LocalPipe::Write(const char *buffer, UInt32 size)
	{
		WriteCompletion completion(this->writePipe.Handle(), buffer, size);
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->writePipe.Handle()), completion);
	}

	void LocalPipe::CloseRead()
	{
		this->readPipe.Close();
	}

	void LocalPipe::CloseWrite()
	{
		this->writePipe.Close();
	}
}
