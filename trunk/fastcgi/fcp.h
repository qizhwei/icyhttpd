#ifndef _FCP_H
#define _FCP_H

#include "fc.h"
#include "fastcgi.h"
#include "ob.h"
#include "win32.h"

typedef struct _FcpProcess FcpProcess;

typedef struct _FcpWaitBlock {
	FcpProcess *Process;
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

struct _FcpProcess {
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
	FcpProcess *Process;
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
extern ObDirectoryObject * FcpPoolDirectory;

extern FcpProcess * FcpCreateProcess(FcPool *pool);
extern int FcpTerminateProcess(FcpProcess *process, int error);
extern int FcpPushPoolProcess(FcpProcess *process);
extern void FcpRemovePoolProcess(FcpProcess *process);
extern FcpProcess * FcpPopPoolProcess(FcPool *pool);
extern int FcpDispatchProcess(FcpProcess *process);
extern int FcpWriteProcess(FcpProcess *process, const void *buffer, size_t size, RtlIoCompletion *completion, void *state);
extern void FcpProcessClose(void *object);
extern void FcpRequestClose(void *object);
extern void FcpWaitBlockClose(void *object);

#endif
