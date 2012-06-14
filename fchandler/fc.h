#ifndef _FC_H
#define _FC_H

#include <fcpool.h>
#include <httpd_daemon.h>

#define FC_INITIAL_ALLOC_SIZE (8192)
#define FC_BLOCK_SIZE (4096)
#define FC_READ_SIZE (4096)
#define FC_READER_MAX_ALLOC_SIZE (65536)

typedef struct _FC_HANDLER {
	DM_HANDLER Handler;
	fc_pool_t *Pool;
} FC_HANDLER;

// The following wrapper functions can be called in library thread

extern void FcClose(
	void *Object);

extern void FcAbortRequest(
	fc_request_t *Request);

extern CSTATUS FcBeginRequest(
	fc_request_t *Request,
	fc_pool_t *Pool);

extern CSTATUS FcReadRequest(
	fc_request_t *Request,
	char *Buffer,
	size_t Size,
	OUT size_t *ActualSize);

extern CSTATUS FcWriteRequest(
	fc_request_t *Request,
	const char *Buffer,
	size_t Size,
	OUT size_t *ActualSize);

extern CSTATUS FcWriteParamRequest(
	fc_request_t *Request,
	const char *Buffer,
	size_t Size,
	OUT size_t *ActualSize);

#endif
