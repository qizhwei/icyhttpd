#include <win32_common.h>
#include <httpd_util.h>
#include <stdio.h>

// TODO

CSTATUS Win32ErrorCodeToCStatus(DWORD dwErrorCode)
{
	switch (dwErrorCode) {
	case ERROR_SUCCESS:
		return C_SUCCESS;
	case ERROR_HANDLE_EOF:
		return C_END_OF_FILE;
	case ERROR_ACCESS_DENIED:
		return C_ACCESS_DENIED;
	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
		return C_NOT_FOUND;
	case ERROR_NETNAME_DELETED:
		return C_CONNECTION_BROKEN;
	default:
		LogPrintf("Warning: unknown win32 error %u, C_OS_ERROR is used.\n", (unsigned int)dwErrorCode);
		return C_OS_ERROR;
	}
}

CSTATUS WSAErrorCodeToCStatus(
	int iErrorCode)
{
	switch (iErrorCode) {
	case 0:
		return C_SUCCESS;
	case WSAECONNABORTED:
	case WSAECONNRESET:
		return C_CONNECTION_BROKEN;
	default:
		LogPrintf("Warning: unknown winsock error %d, C_SOCKET_ERROR is used.\n", iErrorCode);
		return C_SOCKET_ERROR;
	}
}
