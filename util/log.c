#include <stdio.h>
#include <stdarg.h>
#include <httpd_util.h>

CSTATUS LogVPrintf(
	const char *Format,
	va_list ArgList)
{
	int result;

	result = vfprintf(stderr, Format, ArgList);
	if (result < 0)
		return ErrnoToCStatus(errno);

	return C_SUCCESS;
}

CSTATUS LogPrintf(
	const char *Format, ...)
{
	CSTATUS status;
	va_list args;

	va_start(args, Format);
	status = LogVPrintf(Format, args);
	va_end(args);

	return status;
}
