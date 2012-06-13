#ifndef _NAIVEIO_H
#define _NAIVEIO_H

#include <win32_common.h>
#include <httpd_io.h>

struct _IO_THREAD {
	volatile LONG ReferenceCount;
	IO_THREAD_ENTRY *ThreadEntry;
	void *UserContext;
	HANDLE ThreadHandle;
	HANDLE EventHandle;
};

typedef struct _IOP_APC_BLOCK {
	IO_APC_ENTRY *ApcEntry;
	void *Context;
	HANDLE EventHandle;
} IOP_APC_BLOCK;

extern LPFN_TRANSMITFILE IopfnTransmitFile;
extern HANDLE IopRunEvent;
extern DWORD IopTlsIndex;
extern HANDLE IopApcThread;

#define CURRENT_THREAD() \
	((IO_THREAD *)TlsGetValue(IopTlsIndex))

#endif
