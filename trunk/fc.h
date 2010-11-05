#ifndef _FC_H
#define _FC_H

#include "rtl.h"

typedef struct _FcPool FcPool;
typedef struct _FcProcess FcProcess;
typedef struct _FcRequest FcRequest;

typedef void FcBeginRequestCompletion(void *state, FcRequest *request, int error);

extern int FcInitializeSystem(void);
extern FcPool * FcCreatePool(const char *commandLine, int idleTime, int maxRequests);
extern int FcBeginRequest(FcPool *pool, const char *scriptPath, FcBeginRequestCompletion *completion, void *state);
extern int FcReadRequest(FcRequest *request, char *buffer, size_t size, RtlIoCompletion *completion, void *state);
extern int FcWriteRequest(FcRequest *request, char *buffer, size_t size, RtlIoCompletion *completion, void *state);

#endif
