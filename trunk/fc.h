#ifndef _FC_H
#define _FC_H

#include <stddef.h>

typedef struct _FcPool FcPool;
typedef struct _FcProcess FcProcess;
typedef struct _FcRequest FcRequest;

typedef void FcBeginRequestCompletion(void *state, FcRequest *request, int error);
typedef void FcReadWriteCompletion(void *state, size_t size, int error);

extern FcPool * FcCreatePool(const char *commandLine, int idleTime);
extern int FcBeginRequest(FcPool *pool, const char *scriptPath, FcBeginRequestCompletion *completion, void *state);
extern int FcReadRequest(FcRequest *request, char *buffer, size_t size, FcReadWriteCompletion *completion, void *state);
extern int FcWriteRequest(FcRequest *request, char *buffer, size_t size, FcReadWriteCompletion *completion, void *state);
extern void FcEndRequest(FcRequest *request);

#endif
