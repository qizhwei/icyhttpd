#ifndef _FC_H
#define _FC_H

#include "rtl.h"

typedef struct _FcPool FcPool;
typedef struct _FcProcess FcProcess;
typedef struct _FcRequest FcRequest;

extern int FcInitializeSystem(void);
extern FcPool * FcCreatePool(const char *commandLine, int idleTime, int maxRequests);
extern FcRequest * FcBeginRequest(FcPool *pool, const char *scriptPath);
extern int FcReadRequest(FcRequest *request, char *buffer, size_t size, RtlIoCompletion *completion, void *state);
extern int FcWriteRequest(FcRequest *request, char *buffer, size_t size, RtlIoCompletion *completion, void *state);

#endif
