#include <win32_common.h>
#include <httpd_util.h>

CSTATUS Utf8ToUtf16GetSize(
	OUT size_t *SizeInChars,
	const char *Input)
{
	int result = MultiByteToWideChar(CP_UTF8, 0, Input, -1, NULL, 0);
	if (result == 0)
		return Win32ErrorCodeToCStatus(GetLastError());
	*SizeInChars = result;
	return C_SUCCESS;
}

CSTATUS Utf8ToUtf16(
	OUT wchar_t *Buffer,
	size_t BufferSizeInChars,
	const char *Input)
{
	int result = MultiByteToWideChar(CP_UTF8, 0, Input, -1, Buffer, (int)BufferSizeInChars);
	if (result == 0)
		return Win32ErrorCodeToCStatus(GetLastError());
	return C_SUCCESS;
}

CSTATUS AnsiToUtf16GetSize(
	OUT size_t *SizeInChars,
	const char *Input)
{
	int result = MultiByteToWideChar(CP_ACP, 0, Input, -1, NULL, 0);
	if (result == 0)
		return Win32ErrorCodeToCStatus(GetLastError());
	*SizeInChars = result;
	return C_SUCCESS;
}

CSTATUS AnsiToUtf16(
	OUT wchar_t *Buffer,
	size_t BufferSizeInChars,
	const char *Input)
{
	int result = MultiByteToWideChar(CP_ACP, 0, Input, -1, Buffer, (int)BufferSizeInChars);
	if (result == 0)
		return Win32ErrorCodeToCStatus(GetLastError());
	return C_SUCCESS;
}
