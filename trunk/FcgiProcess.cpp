#include "FcgiProcess.h"
#include "Exception.h"
#include "Utility.h"
#include "Dispatcher.h"
#include "Pipe.h"
#include <string>
#include <cstdio>

using namespace Httpd;
using namespace std;

namespace Httpd
{
	FcgiProcess::FcgiProcess(const wstring &commandLine, UInt32 maxRequests)
	{
		STARTUPINFOW si = {0};
		PROCESS_INFORMATION pi;
		HANDLE hPipe[2];

		CreatePipePairDuplex(hPipe);

		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = hPipe[0];
		si.hStdOutput = INVALID_HANDLE_VALUE;
		si.hStdError = INVALID_HANDLE_VALUE;

		if (!CreateProcessW(NULL, const_cast<LPWSTR>(commandLine.c_str()),
			NULL, NULL, TRUE, CREATE_BREAKAWAY_FROM_JOB, NULL, NULL, &si, &pi))
		{
			CloseHandle(hPipe[0]);
			CloseHandle(hPipe[1]);
			throw SystemException();
		}

		CloseHandle(hPipe[0]);
		CloseHandle(pi.hThread);

		this->hProcess = pi.hProcess;
		this->hPipe = hPipe[1];

		Dispatcher::Instance().Queue(ReadPipeCallback, this->AddRef());
	}

	void FcgiProcess::ReadPipeCallback(void *param)
	{
		FcgiProcess *fcp = static_cast<FcgiProcess *>(param);
		Pipe pipe(fcp->hPipe);

		char buffer[4096];

		printf("read begin.\n");

		while (pipe.Read(buffer, sizeof(buffer)) != 0) {
			printf("read sth..\n");
		}

		printf("read eof!\n");
		fcp->Release();
	}

	FcgiProcess::~FcgiProcess()
	{
		TerminateProcess(this->hProcess, 1);
		CloseHandle(this->hPipe);
		CloseHandle(this->hProcess);
	}
}
