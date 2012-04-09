#ifndef _WIN32_COMMON_H
#define _WIN32_COMMON_H

#include <win32.h>
#include <cstatus.h>

extern CSTATUS Win32ErrorCodeToCStatus(
	DWORD dwErrorCode);

extern CSTATUS WSAErrorCodeToCStatus(
	int iErrorCode);

#endif
