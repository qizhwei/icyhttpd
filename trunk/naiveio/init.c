#include "naiveio.h"

LPFN_TRANSMITFILE IopfnTransmitFile;
HANDLE IopRunEvent;

CSTATUS IoInitSystem(void)
{
	int result;
	CSTATUS status;
	WSADATA WSAData;
	SOCKET s;
	DWORD dummy;
	static const GUID guidTransmitFile = WSAID_TRANSMITFILE;

	IopRunEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (!IopRunEvent)
		return Win32ErrorCodeToCStatus(GetLastError());

	result = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (result) {
		CloseHandle(IopRunEvent);
		return WSAErrorCodeToCStatus(result);
	}

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		status = WSAErrorCodeToCStatus(WSAGetLastError());
		WSACleanup();
		CloseHandle(IopRunEvent);
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
	if (!SetEvent(IopRunEvent))
		return Win32ErrorCodeToCStatus(GetLastError());

	while (1)
		SleepEx(INFINITE, TRUE);
}
