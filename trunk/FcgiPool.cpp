#include "FcgiPool.h"
#include "Exception.h"
#include "Win32.h"
#include "Constant.h"

namespace Httpd
{
	FcgiPool::FcgiPool(const std::wstring &commandLine, UInt32 idleTime, UInt32 maxRequests)
		: commandLine(commandLine), idleTime(idleTime), maxRequests(maxRequests)
	{
		if ((hQueue = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL
			|| (hJob = CreateJobObjectW(NULL, NULL)) == NULL)
			throw FatalException();

		for (int i = 0; i < FcgiPoolThreadCount; ++i) {
			HANDLE hThread = CreateThread(NULL, StackReserveSize, &ThreadCallback, this, 0, NULL);
			if (hThread == NULL)
				throw FatalException();
			CloseHandle(hThread);
		}
	}

	DWORD WINAPI FcgiPool::ThreadCallback(LPVOID lpParameter)
	{
		FcgiPool *fcp = reinterpret_cast<FcgiPool *>(lpParameter);

		DWORD NumberOfBytes;
		ULONG_PTR CompletionKey;
		LPOVERLAPPED lpOverlapped;
		BOOL bSuccess;
		
		while (true) {
			bSuccess = GetQueuedCompletionStatus(fcp->hQueue,
				&NumberOfBytes, &CompletionKey, &lpOverlapped, INFINITE);

			if (bSuccess == FALSE && lpOverlapped == NULL)
				throw FatalException();


		}
	}

}
