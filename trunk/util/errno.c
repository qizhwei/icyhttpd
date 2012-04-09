#include <errno.h>
#include <httpd_util.h>

// TODO

CSTATUS ErrnoToCStatus(int errno_)
{
	switch (errno_) {
	case 0:
		return C_SUCCESS;
	default:
		LogPrintf("Warning: unknown libc error %d, C_LIBC_ERROR is used.\n", errno_);
		return C_LIBC_ERROR;
	}
}
