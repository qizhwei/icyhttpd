#include "fiberio.h"

static CSTATUS IopCreateSocket(
	OUT SOCKET *Socket)
{
	SOCKET s;
	CSTATUS status;

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		return WSAErrorCodeToCStatus(WSAGetLastError());

	SetHandleInformation((HANDLE)s, HANDLE_FLAG_INHERIT, 0);

	if (CreateIoCompletionPort((HANDLE)s, IopQueueHandle, FIBERIO_COMPLETE_KEY, 0) == NULL) {
		status = Win32ErrorCodeToCStatus(GetLastError());
		closesocket(s);
		return status;
	}

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

#define ADDRESS_LENGTH (sizeof(struct sockaddr_in6) + 16)

typedef struct _ACCEPT_BLOCK {
	IOP_IO_BLOCK IoBlock;
	SOCKET ListenerSocket;
	SOCKET ClientSocket;
	char Buffer[ADDRESS_LENGTH * 2];
} ACCEPT_BLOCK;

static CSTATUS IopCreateClientByAcceptDelayedIo(
	IOP_IO_BLOCK *IoBlock)
{
	ACCEPT_BLOCK *block = CONTAINING_RECORD(IoBlock, ACCEPT_BLOCK, IoBlock);
	int iErrorCode;

	if (!IopfnAcceptEx(block->ListenerSocket, block->ClientSocket,
		block->Buffer, 0, ADDRESS_LENGTH, ADDRESS_LENGTH, NULL, &IoBlock->Overlapped))
	{
		iErrorCode = WSAGetLastError();
		if (iErrorCode == WSA_IO_PENDING)
			return C_SUCCESS;
		return WSAErrorCodeToCStatus(iErrorCode);
	}

	return C_SUCCESS;
}

CSTATUS IoCreateClientByAccept(
	OUT IO_CLIENT **Client,
	IO_LISTENER *Listener)
{
	ACCEPT_BLOCK block;
	CSTATUS status;
	size_t dummy;

	INIT_IO_BLOCK(&block.IoBlock, IopCreateClientByAcceptDelayedIo);

	block.ListenerSocket = (SOCKET)Listener;
	status = IopCreateSocket(&block.ClientSocket);
	if (!SUCCESS(status))
		return status;

	status = IopDispatcherBlock(&block.IoBlock, (HANDLE)Listener, &dummy);
	if (!SUCCESS(status)) {
		closesocket(block.ClientSocket);
		return status;
	}

	*Client = (IO_CLIENT *)block.ClientSocket;
	return C_SUCCESS;
}

typedef struct _CONNECT_BLOCK {
	IOP_IO_BLOCK IoBlock;
	SOCKET Socket;
	struct sockaddr_in Service;
} CONNECT_BLOCK;

static CSTATUS IopCreateClientByConnectDelayedIo(
	IOP_IO_BLOCK *IoBlock)
{
	CONNECT_BLOCK *block = CONTAINING_RECORD(IoBlock, CONNECT_BLOCK, IoBlock);
	int iErrorCode;

	if (!IopfnConnectEx(block->Socket, (SOCKADDR *)&block->Service, sizeof(block->Service),
		NULL, 0, 0, &IoBlock->Overlapped))
	{
		iErrorCode = WSAGetLastError();
		if (iErrorCode == WSA_IO_PENDING)
			return C_SUCCESS;
		return WSAErrorCodeToCStatus(iErrorCode);
	}

	return C_SUCCESS;
}

CSTATUS IoCreateClientByConnect(
	OUT IO_CLIENT **Client,
	const char *IPAddress,
	int Port)
{
	CONNECT_BLOCK block;
	struct sockaddr_in service;
	CSTATUS status;
	size_t dummy;

	INIT_IO_BLOCK(&block.IoBlock, IopCreateClientByConnectDelayedIo);

	if (Port <= 0 || Port > USHRT_MAX)
		return C_INVALID_PORT;

	block.Service.sin_family = AF_INET;
	block.Service.sin_port = htons(Port);
	block.Service.sin_addr.s_addr = inet_addr(IPAddress);

	if (block.Service.sin_addr.s_addr == INADDR_NONE)
		return C_INVALID_IP_ADDRESS;

	status = IopCreateSocket(&block.Socket);
	if (!SUCCESS(status))
		return status;

	service.sin_family = AF_INET;
	service.sin_port = 0;
	service.sin_addr.s_addr = INADDR_ANY;

	if (bind(block.Socket, (SOCKADDR *)&service, sizeof(service))) {
		status = WSAErrorCodeToCStatus(WSAGetLastError());
		closesocket(block.Socket);
		return status;
	}

	status = IopDispatcherBlock(&block.IoBlock, (HANDLE)block.Socket, &dummy);
	if (!SUCCESS(status)) {
		closesocket(block.Socket);
		return status;
	}

	*Client = (IO_CLIENT *)block.Socket;
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
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
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

typedef struct _READ_WRITE_BLOCK {
	IOP_IO_BLOCK IoBlock;
	SOCKET Socket;
	WSABUF WSABuf;
} READ_WRITE_BLOCK;

static CSTATUS IopReadClientDelayedIo(
	IOP_IO_BLOCK *IoBlock)
{
	READ_WRITE_BLOCK *block = CONTAINING_RECORD(IoBlock, READ_WRITE_BLOCK, IoBlock);
	DWORD dwFlags = 0;
	int iErrorCode;

	if (WSARecv(block->Socket, &block->WSABuf, 1, NULL, &dwFlags,
		&IoBlock->Overlapped, NULL))
	{
		iErrorCode = WSAGetLastError();
		if (iErrorCode == WSA_IO_PENDING)
			return C_SUCCESS;
		return WSAErrorCodeToCStatus(iErrorCode);
	}

	return C_SUCCESS;
}

CSTATUS IoReadClient(
	IO_CLIENT *Client,
	char *Buffer,
	size_t Size,
	OUT size_t *ActualSize)
{
	READ_WRITE_BLOCK block;
	CSTATUS status;
	size_t size;

	INIT_IO_BLOCK(&block.IoBlock, IopReadClientDelayedIo);

	block.Socket = (SOCKET)Client;
	block.WSABuf.buf = Buffer;
	block.WSABuf.len = (ULONG)Size;

	status = IopDispatcherBlock(&block.IoBlock, (HANDLE)Client, &size);
	if (!SUCCESS(status))
		return status;

	*ActualSize = (size_t)size;
	return C_SUCCESS;
}

static CSTATUS IopWriteClientDelayedIo(
	IOP_IO_BLOCK *IoBlock)
{
	READ_WRITE_BLOCK *block = CONTAINING_RECORD(IoBlock, READ_WRITE_BLOCK, IoBlock);
	int iErrorCode;

	if (WSASend(block->Socket, &block->WSABuf, 1, NULL, 0,
		&IoBlock->Overlapped, NULL))
	{
		iErrorCode = WSAGetLastError();
		if (iErrorCode == WSA_IO_PENDING)
			return C_SUCCESS;
		return WSAErrorCodeToCStatus(iErrorCode);
	}

	return C_SUCCESS;
}

CSTATUS IoWriteClient(
	IO_CLIENT *Client,
	const char *Buffer,
	size_t Size,
	OUT size_t *ActualSize)
{
	READ_WRITE_BLOCK block;
	CSTATUS status;
	size_t size;

	INIT_IO_BLOCK(&block.IoBlock, IopWriteClientDelayedIo);

	block.Socket = (SOCKET)Client;
	block.WSABuf.buf = (CHAR *)Buffer;
	block.WSABuf.len = (ULONG)Size;

	status = IopDispatcherBlock(&block.IoBlock, (HANDLE)Client, &size);
	if (!SUCCESS(status))
		return status;

	*ActualSize = (size_t)size;
	return C_SUCCESS;
}

typedef struct _TRANSMIT_FILE_BLOCK {
	IOP_IO_BLOCK IoBlock;
	SOCKET Socket;
	HANDLE File;
	DWORD Size;
} TRANSMIT_FILE_BLOCK;

CSTATUS IopTransmitFileClientDelayedIo(
	IOP_IO_BLOCK *IoBlock)
{
	TRANSMIT_FILE_BLOCK *block = CONTAINING_RECORD(IoBlock, TRANSMIT_FILE_BLOCK, IoBlock);
	int iErrorCode;

	if (!IopfnTransmitFile(block->Socket, block->File, block->Size, 0,
		&IoBlock->Overlapped, NULL, 0))
	{
		iErrorCode = WSAGetLastError();
		if (iErrorCode == WSA_IO_PENDING)
			return C_SUCCESS;
		return WSAErrorCodeToCStatus(iErrorCode);
	}

	return C_SUCCESS;
}

CSTATUS IopTransmitFileClient(
	IO_CLIENT *Client,
	IO_FILE *File,
	DWORD Size)
{
	TRANSMIT_FILE_BLOCK block;
	size_t dummy;

	INIT_IO_BLOCK(&block.IoBlock, IopTransmitFileClientDelayedIo);

	block.Socket = (SOCKET)Client;
	block.File = (HANDLE)File;
	block.Size = Size;

	return IopDispatcherBlock(&block.IoBlock, (HANDLE)Client, &dummy);
}

CSTATUS IoTransmitFileClient(
	IO_CLIENT *Client,
	IO_FILE *File,
	uint64_t Offset,
	uint64_t Length)
{
	LARGE_INTEGER liOffset;
	CSTATUS status;

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

		status = IopTransmitFileClient(Client, File, size);
		if (!SUCCESS(status))
			return status;

		Length -= size;
	}

	return C_SUCCESS;
}
