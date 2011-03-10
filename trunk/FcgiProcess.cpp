#include "FcgiProcess.h"
#include "Fastcgi.h"
#include "Exception.h"
#include "Utility.h"
#include "Pipe.h"
#include "Win32.h"
#include <string>
#include <utility>

using namespace Httpd;
using namespace std;

namespace Httpd
{
	FcgiProcess *FcgiProcess::Create(const wstring &commandLine, UInt16 maxRequests)
	{
		STARTUPINFOW si = {0};
		PROCESS_INFORMATION pi;
		auto hPipes = CreatePipePairDuplex();

		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = hPipes.first;
		si.hStdOutput = INVALID_HANDLE_VALUE;
		si.hStdError = INVALID_HANDLE_VALUE;

		if (!CreateProcessW(NULL, const_cast<LPWSTR>(commandLine.c_str()),
			NULL, NULL, TRUE, CREATE_BREAKAWAY_FROM_JOB, NULL, NULL, &si, &pi))
		{
			CloseHandle(hPipes.first);
			CloseHandle(hPipes.second);
			throw SystemException();
		}

		CloseHandle(hPipes.first);
		CloseHandle(pi.hThread);

		return new FcgiProcess(pi.hProcess, hPipes.second, maxRequests);
	}

	FcgiProcess::FcgiProcess(HANDLE hProcess, HANDLE hPipe, LONG maxRequests)
		: process(hProcess), pipe(hPipe), remainingRequests(maxRequests)
	{}

	FcgiProcess::~FcgiProcess()
	{
		TerminateProcess(this->process.Handle(), 1);
	}

	UInt16 FcgiProcess::Acquire()
	{
		UInt16 requestId = static_cast<UInt16>(InterlockedDecrement(&remainingRequests));
		FcgiBeginRequestRecord fb(requestId, FcgiResponder, FcgiKeepConn);
		this->Write(fb.Buffer(), sizeof(fb));
		return requestId;
	}

	UInt32 FcgiProcess::Read(char *buffer, UInt32 size)
	{
		return pipe.Read(buffer, size);
	}

	void FcgiProcess::Write(const char *buffer, UInt32 size)
	{
		pipe.Write(buffer, size);
	}
}
