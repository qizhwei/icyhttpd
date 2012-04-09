#ifndef _FIBERIO_H
#define _FIBERIO_H

#include <httpd_io.h>
#include <win32_common.h>

#define FIBERIO_THREAD_COUNT (8)
#define FIBERIO_THREAD_STACK_SIZE (0)
#define FIBERIO_FIBER_STACK_COMMIT_SIZE (4096)
#define FIBERIO_FIBER_STACK_RESERVE_SIZE (16384)

#define FIBERIO_SWITCH_KEY (0)
#define FIBERIO_COMPLETE_KEY (1)

typedef struct _IOP_IO_BLOCK IOP_IO_BLOCK;
typedef CSTATUS DELAYED_IO_FUNCTION(IOP_IO_BLOCK *IoBlock);

typedef struct _IOP_IO_BLOCK {
	OVERLAPPED Overlapped;
	LPVOID FiberHandle;
	DELAYED_IO_FUNCTION *DelayedIoFunction;
} IOP_IO_BLOCK;

#define INIT_IO_BLOCK(iob, func) \
	do { \
		memset(&(iob)->Overlapped, 0, sizeof(OVERLAPPED)); \
		(iob)->FiberHandle = GetCurrentFiber(); \
		(iob)->DelayedIoFunction = (func); \
	} while (0)

typedef struct _IOP_THREAD_BLOCK {
	HANDLE ThreadHandle;
	LPVOID MainFiberHandle;
	IOP_IO_BLOCK *DelayedIoBlock;
	CSTATUS DelayedIoStatus;
} IOP_THREAD_BLOCK;

struct _IO_THREAD {
	volatile LONG ReferenceCount;
	IO_THREAD_ENTRY *ThreadEntry;
	void *UserContext;
	LPVOID FiberHandle;
	IO_THREAD *JoinThread;
};

extern LPFN_ACCEPTEX IopfnAcceptEx;
extern LPFN_TRANSMITFILE IopfnTransmitFile;

extern int IopThreadCount;
extern DWORD IopTlsIndex;
extern HANDLE IopQueueHandle;
extern IOP_THREAD_BLOCK *IopThreadBlocks;

extern DWORD CALLBACK IopDispatcherThreadEntry(
	LPVOID Context);

extern CSTATUS IopDispatcherBlock(
	IOP_IO_BLOCK *IoBlock,
	HANDLE ObjectHandle,
	OUT size_t *ActualSize);

#define CURRENT_THREAD_BLOCK() \
	((IOP_THREAD_BLOCK *)TlsGetValue(IopTlsIndex))

#endif
