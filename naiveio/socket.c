#include <limits.h>
#include "naiveio.h"

static CSTATUS IopCreateSocket(
	OUT SOCKET *Socket)
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		return WSAErrorCodeToCStatus(WSAGetLastError());

	SetHandleInformation((HANDLE)s, HANDLE_FLAG_INHERIT, 0);
	*Socket = s;
	return C_SUCCESS;
}

CSTATUS IoCreateListener(
	OUT IO_LISTENER **Listener,
	const char *IPAddress,
	int Port)
{
	struct sockaddr_in service = {0};
	SOCKET s;
	CSTATUS status;

	if (Port <= 0 || Port > USHRT_MAX)
		return C_INVALID_PORT;

	service.sin_family = AF_INET;
	service.sin_port = htons(Port);
	service.sin_addr.s_addr = inet_addr(IPAddress);

	if (service.sin_addr.s_addr == INADDR_NONE)
		return C_INVALID_IP_ADDRESS;

	status = IopCreateSocket(&s);
	if (!SUCCESS(status))
		return status;

	if (bind(s, (SOCKADDR *)&service, sizeof(service))) {
		status = WSAErrorCodeToCStatus(WSAGetLastError());
		closesocket(s);
		return status;
	}

	if (listen(s, SOMAXCONN)) {
		status = WSAErrorCodeToCStatus(WSAGetLastError());
		closesocket(s);
		return status;
	}

	*Listener = (IO_LISTENER *)s;
	return C_SUCCESS;
}

void IoDestroyListener(
	IO_LISTENER *Listener)
{
	closesocket((SOCKET)Listener);
}

CSTATUS IoCreateClientByAccept(
	OUT IO_CLIENT **Client,
	IO_LISTENER *Listener)
{
	SOCKET s;

	s = accept((SOCKET)Listener, NULL, NULL);
	if (s == INVALID_SOCKET)
		return WSAErrorCodeToCStatus(WSAGetLastError());

	*Client = (IO_CLIENT *)s;
	return C_SUCCESS;
}

CSTATUS IoCreateClientByConnect(
	OUT IO_CLIENT **Client,
	const char *IPAddress,
	int Port)
{
	SOCKET s;
	CSTATUS status;
	struct sockaddr_in service = {0};

	if (Port <= 0 || Port > USHRT_MAX)
		return C_INVALID_PORT;

	service.sin_family = AF_INET;
	service.sin_port = htons(Port);
	service.sin_addr.s_addr = inet_addr(IPAddress);

	if (service.sin_addr.s_addr == INADDR_NONE)
		return C_INVALID_IP_ADDRESS;

	status = IopCreateSocket(&s);
	if (!SUCCESS(status))
		return status;

	if (connect(s, (SOCKADDR *)&service, sizeof(service))) {
		status = WSAErrorCodeToCStatus(WSAGetLastError());
		closesocket(s);
		return status;
	}

	*Client = (IO_CLIENT *)s;
	return C_SUCCESS;
}

void IoDestroyClient(
	IO_CLIENT *Client)
{
	closesocket((SOCKET)Client);
}

CSTATUS IoCreateFile(
	OUT IO_FILE **File,
	const wchar_t *Path)
{
	HANDLE hFile;
	
	hFile = CreateFileW(Path, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return Win32ErrorCodeToCStatus(GetLastError());

	*File = (IO_FILE *)hFile;
	return C_SUCCESS;
}

CSTATUS IoGetSizeFile(
	IO_FILE *File,
	OUT uint64_t *FileSize)
{
	LARGE_INTEGER liFileSize;

	if (!GetFileSizeEx((HANDLE)File, &liFileSize))
		return Win32ErrorCodeToCStatus(GetLastError());

	*FileSize = (uint64_t)liFileSize.QuadPart;
	return C_SUCCESS;
}

void IoDestroyFile(
	IO_FILE *File)
{
	CloseHandle((HANDLE)File);
}

CSTATUS IoReadClient(
	IO_CLIENT *Client,
	char *Buffer,
	size_t Size,
	OUT size_t *ActualSize)
{
	WSABUF wb;
	DWORD recvd;
	DWORD flags = 0;

	wb.buf = Buffer;
	wb.len = Size;

	if (WSARecv((SOCKET)Client, &wb, 1, &recvd, &flags, NULL, NULL))
		return WSAErrorCodeToCStatus(WSAGetLastError());

	*ActualSize = recvd;
	return C_SUCCESS;
}

CSTATUS IoWriteClient(
	IO_CLIENT *Client,
	const char *Buffer,
	size_t Size,
	OUT size_t *ActualSize)
{
	WSABUF wb;
	DWORD sent;

	wb.buf = (char *)Buffer;
	wb.len = Size;

	if (WSASend((SOCKET)Client, &wb, 1, &sent, 0, NULL, NULL))
		return WSAErrorCodeToCStatus(WSAGetLastError());

	*ActualSize = sent;
	return C_SUCCESS;
}

CSTATUS IoTransmitFileClient(
	IO_CLIENT *Client,
	IO_FILE *File,
	uint64_t Offset,
	uint64_t Length)
{
	LARGE_INTEGER liOffset;

	liOffset.QuadPart = Offset;

	if (!SetFilePointerEx((HANDLE)File, liOffset, NULL, FILE_BEGIN))
		return Win32ErrorCodeToCStatus(GetLastError());

	while (Length) {
		DWORD size;

#define TRANSMIT_MAX (0x80000000)
		if (Length >= TRANSMIT_MAX) {
			size = TRANSMIT_MAX;
		} else {
			size = (DWORD)Length;
		}

		if (!IopfnTransmitFile((SOCKET)Client, (HANDLE)File, size, 0, NULL, NULL, 0))
			return WSAErrorCodeToCStatus(WSAGetLastError());

		Length -= size;
	}

	return C_SUCCESS;
}
