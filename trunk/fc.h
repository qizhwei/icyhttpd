#ifndef _FC_H
#define _FC_H

#include "rtl.h"
#include "fs.h"

typedef struct _FcPool FcPool;
typedef struct _FcRequest FcRequest;

typedef struct _FcRequestParam {
	const char *ScriptFilename;
	const char *RequestMethod;
	const char *ContentLength;
	const char *ContentType;
} FcRequestParam;

extern int FcInitializeSystem(void);
extern FcPool * FcCreatePool(const char *name, const char *commandLine, int idleTime, int maxRequests);
extern FcRequest * FcBeginRequest(FcPool *pool, FcRequestParam *param);
extern int FcReadRequest(FcRequest *request, char *buffer, size_t size, RtlIoCompletion *completion, void *state);
extern int FcWriteRequest(FcRequest *request, char *buffer, size_t size, RtlIoCompletion *completion, void *state);

#endif
