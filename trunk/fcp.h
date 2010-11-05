#ifndef _FCP_H
#define _FCP_H

#include "fc.h"
#include "fastcgi.h"
#include "ob.h"
#include "win32.h"

typedef struct _FcpWaitBlock {
	FcProcess *Process;
	HANDLE Timer;
	int Cancelled;
} FcpWaitBlock;

struct _FcPool {
	ListEntry RunningList;
	ListEntry PoolingList;
	char *CommandLine;
	int IdleTime;
	int MaxRequests;
};

struct _FcProcess {
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
	FcProcess *Process;
	RtlFifo *StdinFifo;
	RtlFifo *StdoutFifo;
};

#define FCP_STATE_READY (0)
#define FCP_STATE_INTERACTIVE (1)
#define FCP_STATE_POOLING (2)
#define FCP_STATE_TERMINATED (3)

extern HANDLE FcpJobObject;
extern ObObjectType FcpPoolObjectType;
extern ObObjectType FcpProcessObjectType;
extern ObObjectType FcpRequestObjectType;
extern ObObjectType FcpWaitBlockObjectType;

extern FcProcess * FcpCreateProcess(FcPool *pool);
extern int FcpTerminateProcess(FcProcess *process, int error);
extern int FcpPushPoolProcess(FcProcess *process);
extern void FcpRemovePoolProcess(FcProcess *process);
extern FcProcess * FcpPopPoolProcess(FcPool *pool);
extern int FcpReadPipe(FcProcess *process);
extern int FcpWriteProcess(FcProcess *process, const void *buffer, size_t size, RtlIoCompletion *completion, void *state);
extern void FcpClosePool(void *object);
extern void FcpCloseProcess(void *object);
extern void FcpCloseRequest(void *object);
extern void FcpCloseWaitBlock(void *object);

#endif
