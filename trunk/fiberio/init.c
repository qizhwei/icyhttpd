#include <stdlib.h>
#include "fiberio.h"

LPFN_ACCEPTEX IopfnAcceptEx;
LPFN_CONNECTEX IopfnConnectEx;
LPFN_TRANSMITFILE IopfnTransmitFile;
HANDLE IopApcThread;

CSTATUS IoInitSystem(void)
{
	CSTATUS status;
	int result;
	WSADATA WSAData;
	SOCKET s;
	DWORD dummy;
	int i;
	static const GUID guidAcceptEx = WSAID_ACCEPTEX;
	static const GUID guidConnectEx = WSAID_CONNECTEX;
	static const GUID guidTransmitFile = WSAID_TRANSMITFILE;

	result = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (result)
		return WSAErrorCodeToCStatus(result);

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		status = WSAErrorCodeToCStatus(WSAGetLastError());
		WSACleanup();
		return status;
	}

	if (WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
		(LPVOID)&guidAcceptEx, sizeof(guidAcceptEx),
		&IopfnAcceptEx, sizeof(IopfnAcceptEx), &dummy, NULL, NULL)
		||
		WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
		(LPVOID)&guidConnectEx, sizeof(guidConnectEx),
		&IopfnConnectEx, sizeof(IopfnConnectEx), &dummy, NULL, NULL)
		||
		WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
		(LPVOID)&guidTransmitFile, sizeof(guidTransmitFile),
		&IopfnTransmitFile, sizeof(IopfnTransmitFile), &dummy, NULL, NULL))
	{
		status = WSAErrorCodeToCStatus(WSAGetLastError());
		closesocket(s);
		WSACleanup();
		return status;
	}

	closesocket(s);

	if ((IopTlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES) {
		status = Win32ErrorCodeToCStatus(GetLastError());
		WSACleanup();
		return status;
	}
	
	if ((IopQueueHandle = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
	{
		status = Win32ErrorCodeToCStatus(GetLastError());
		TlsFree(IopTlsIndex);
		WSACleanup();
		return status;
	}

	IopThreadCount = FIBERIO_THREAD_COUNT;
	IopThreadBlocks = (IOP_THREAD_BLOCK *)malloc(sizeof(IOP_THREAD_BLOCK) * IopThreadCount);
	for (i = 0; i < IopThreadCount; ++i) {
		IopThreadBlocks[i].ThreadHandle = CreateThread(NULL, FIBERIO_THREAD_STACK_SIZE,
			IopDispatcherThreadEntry, &IopThreadBlocks[i], CREATE_SUSPENDED, NULL);
		if (IopThreadBlocks[i].ThreadHandle == NULL) {
			status = Win32ErrorCodeToCStatus(GetLastError());

			CloseHandle(IopQueueHandle);
			for (--i; i >= 0; --i) {
				TerminateThread(IopThreadBlocks[i].ThreadHandle, 1);
				CloseHandle(IopThreadBlocks[i].ThreadHandle);
			}

			TlsFree(IopTlsIndex);
			free(IopThreadBlocks);
			WSACleanup();
			return status;
		}
	}

	return C_SUCCESS;
}

void IoUninitSystem(void)
{
	int i;

	CloseHandle(IopQueueHandle);
	for (i = 0; i < IopThreadCount; ++i) {
		WaitForSingleObject(IopThreadBlocks[i].ThreadHandle, INFINITE);
		CloseHandle(IopThreadBlocks[i].ThreadHandle);
	}

	TlsFree(IopTlsIndex);
	free(IopThreadBlocks);
	WSACleanup();
}

CSTATUS IoMainLoop(void)
{
	int i;

	if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
		GetCurrentProcess(), &IopApcThread, 0, FALSE, DUPLICATE_SAME_ACCESS))
		return Win32ErrorCodeToCStatus(GetLastError());

	for (i = 0; i < IopThreadCount; ++i)
		ResumeThread(IopThreadBlocks[i].ThreadHandle);

	while (1)
		SleepEx(INFINITE, TRUE);
}
