#include "naiveio.h"

DWORD IopTlsIndex;
LPFN_TRANSMITFILE IopfnTransmitFile;
HANDLE IopRunEvent;
HANDLE IopApcThread;

CSTATUS IoInitSystem(void)
{
	int result;
	CSTATUS status;
	WSADATA WSAData;
	SOCKET s;
	DWORD dummy;
	static const GUID guidTransmitFile = WSAID_TRANSMITFILE;

	if ((IopTlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES) {
		return Win32ErrorCodeToCStatus(GetLastError());
	}

	IopRunEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (!IopRunEvent) {
		status = Win32ErrorCodeToCStatus(GetLastError());
		TlsFree(IopTlsIndex);
		return status;
	}

	result = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (result) {
		CloseHandle(IopRunEvent);
		TlsFree(IopTlsIndex);
		return WSAErrorCodeToCStatus(result);
	}

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		status = WSAErrorCodeToCStatus(WSAGetLastError());
		WSACleanup();
		CloseHandle(IopRunEvent);
		TlsFree(IopTlsIndex);
		return status;
	}

	if (WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
		(LPVOID)&guidTransmitFile, sizeof(guidTransmitFile),
		&IopfnTransmitFile, sizeof(IopfnTransmitFile), &dummy, NULL, NULL))
	{
		status = WSAErrorCodeToCStatus(WSAGetLastError());
		closesocket(s);
		WSACleanup();
		CloseHandle(IopRunEvent);
		TlsFree(IopTlsIndex);
		return status;
	}

	closesocket(s);

	return C_SUCCESS;
}

void IoUninitSystem(void)
{
	WSACleanup();
	CloseHandle(IopRunEvent);
}

CSTATUS IoMainLoop(void)
{
	if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
		GetCurrentProcess(), &IopApcThread, 0, FALSE, DUPLICATE_SAME_ACCESS))
		return Win32ErrorCodeToCStatus(GetLastError());

	if (!SetEvent(IopRunEvent))
		return Win32ErrorCodeToCStatus(GetLastError());

	while (1)
		SleepEx(INFINITE, TRUE);
}
