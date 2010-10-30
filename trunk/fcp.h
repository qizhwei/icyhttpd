#ifndef _FCP_H
#define _FCP_H

#include "fc.h"
#include "fastcgi.h"
#include "rtl.h"
#include "win32.h"

typedef struct _FcpWaitBlock {
	int ReferenceCount;
	FcProcess *Process;
	HANDLE Timer;
	int Cancelled;
} FcpWaitBlock;

struct _FcPool {
	int ReferenceCount;
	ListEntry RunningList;
	ListEntry PoolingList;
	char *CommandLine;
	int IdleTime;
	int MaxRequests;
};

struct _FcProcess {
	int ReferenceCount;
	int RemainingRequests;
	FcPool *Pool;
	ListEntry PoolEntry;
	int State;
	HANDLE ProcessHandle;
	HANDLE LocalPipe;
	union {
		FcRequest *Request; // State == FCP_STATE_INTERACTIVE
		FcpWaitBlock *WaitBlock; // State == FCP_STATE_POOLING
	};
};

struct _FcRequest {
	int ReferenceCount;
	FcProcess *Process;
	RtlFifo *StdinFifo;
	RtlFifo *StdoutFifo;
};

#define FCP_STATE_READY (0)
#define FCP_STATE_INTERACTIVE (1)
#define FCP_STATE_POOLING (2)
#define FCP_STATE_TERMINATED (3)

extern HANDLE FcpJobObject;
extern int FcpInitialize(void);
extern FcProcess * FcpCreateProcess(FcPool *pool);
extern int FcpTerminateProcess(FcProcess *process, int error);
extern int FcpPushPoolProcess(FcProcess *process);
extern void FcpRemovePoolProcess(FcProcess *process);
extern FcProcess * FcpPopPoolProcess(FcPool *pool);
extern int FcpReadPipe(FcProcess *process);
extern int FcpWriteProcess(FcProcess *process, const void *buffer, size_t size, FcReadWriteCompletion *completion, void *state);
extern void FcpDereferencePool(FcPool *pool);
extern void FcpDereferenceProcess(FcProcess *process);
extern void FcpDereferenceRequest(FcRequest *request);
extern void FcpDereferenceWaitBlock(FcpWaitBlock *waitBlock);

#endif
