#ifndef _NAIVEIO_H
#define _NAIVEIO_H

#include <win32_common.h>
#include <httpd_io.h>

struct _IO_THREAD {
	volatile LONG ReferenceCount;
	IO_THREAD_ENTRY *ThreadEntry;
	void *UserContext;
	HANDLE ThreadHandle;
};

extern LPFN_TRANSMITFILE IopfnTransmitFile;
extern HANDLE IopRunEvent;

#endif
