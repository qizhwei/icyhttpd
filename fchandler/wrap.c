#include <stdlib.h>
#include "fc.h"

static void FcCloseApc(void *Context, IO_APC *Apc)
{
	fc_close(Context);
	IoCompleteApc(Apc);
}

void FcClose(
	void *Object)
{
	if (!SUCCESS(IoInvokeApc(FcCloseApc, Object)))
		abort();
}

static void FcAbortRequestApc(void *Context, IO_APC *Apc)
{
	fc_abort_request((fc_request_t *)Context);
	IoCompleteApc(Apc);
}

void FcAbortRequest(
	fc_request_t *Request)
{
	if (!SUCCESS(IoInvokeApc(FcAbortRequestApc, Request)))
		abort();
}

typedef struct _FC_BEGIN_REQUEST_BLOCK {
	fc_request_t *Request;
	fc_pool_t *Pool;
	IO_APC *Apc;
	int Error;
} FC_BEGIN_REQUEST_BLOCK;

static void FcBeginRequestCallback(void *Context, int Error)
{
	FC_BEGIN_REQUEST_BLOCK *block = (FC_BEGIN_REQUEST_BLOCK *)Context;
	block->Error = Error;
	IoCompleteApc(block->Apc);
}

static void FcBeginRequestApc(void *Context, IO_APC *Apc)
{
	FC_BEGIN_REQUEST_BLOCK *block = (FC_BEGIN_REQUEST_BLOCK *)Context;
	int error;

	block->Apc = Apc;
	error = fc_begin_request(block->Request, block->Pool, FcBeginRequestCallback, block);
	if (error) {
		block->Error = error;
		IoCompleteApc(Apc);
	}
}

CSTATUS FcBeginRequest(
	fc_request_t *Request,
	fc_pool_t *Pool)
{
	FC_BEGIN_REQUEST_BLOCK block;

	block.Request = Request;
	block.Pool = Pool;
	
	if (!SUCCESS(IoInvokeApc(FcBeginRequestApc, &block)))
		abort();

	if (block.Error)
		return C_GENERIC_ERROR;

	return C_SUCCESS;
}

typedef struct _FC_REQUEST_IO_BLOCK {
	fc_request_t *Request;
	char *Buffer;
	size_t Size;
	IO_APC *Apc;
	fc_ssize_t Result;
} FC_REQUEST_IO_BLOCK;

static void FcRequestIoCallback(void *Context, fc_ssize_t Result)
{
	FC_REQUEST_IO_BLOCK *block = (FC_REQUEST_IO_BLOCK *)Context;
	block->Result = Result;
	IoCompleteApc(block->Apc);
}

static void FcReadRequestApc(void *Context, IO_APC *Apc)
{
	FC_REQUEST_IO_BLOCK *block = (FC_REQUEST_IO_BLOCK *)Context;

	block->Apc = Apc;
	if (fc_read_request(block->Request, block->Buffer, block->Size, FcRequestIoCallback, block)) {
		block->Result = -1;
		IoCompleteApc(Apc);
	}
}

CSTATUS FcReadRequest(
	fc_request_t *Request,
	char *Buffer,
	size_t Size,
	OUT size_t *ActualSize)
{
	FC_REQUEST_IO_BLOCK block;
	CSTATUS status;

	block.Request = Request;
	block.Buffer = Buffer;
	block.Size = Size;
	
	status = IoInvokeApc(FcReadRequestApc, &block);
	if (!SUCCESS(status))
		return status;

	if (block.Result < 0)
		return C_GENERIC_ERROR;

	*ActualSize = block.Result;
	return C_SUCCESS;
}

static void FcWriteRequestApc(void *Context, IO_APC *Apc)
{
	FC_REQUEST_IO_BLOCK *block = (FC_REQUEST_IO_BLOCK *)Context;

	block->Apc = Apc;
	if (fc_write_request(block->Request, block->Buffer, block->Size, FcRequestIoCallback, block)) {
		block->Result = -1;
		IoCompleteApc(Apc);
	}
}

CSTATUS FcWriteRequest(
	fc_request_t *Request,
	const char *Buffer,
	size_t Size,
	OUT size_t *ActualSize)
{
	FC_REQUEST_IO_BLOCK block;
	CSTATUS status;

	block.Request = Request;
	block.Buffer = (char *)Buffer;
	block.Size = Size;
	
	status = IoInvokeApc(FcWriteRequestApc, &block);
	if (!SUCCESS(status))
		return status;

	if (block.Result < 0)
		return C_GENERIC_ERROR;

	*ActualSize = block.Result;
	return C_SUCCESS;
}

static void FcWriteParamRequestApc(void *Context, IO_APC *Apc)
{
	FC_REQUEST_IO_BLOCK *block = (FC_REQUEST_IO_BLOCK *)Context;

	block->Apc = Apc;
	if (fc_write_param_request(block->Request, block->Buffer, block->Size, FcRequestIoCallback, block)) {
		block->Result = -1;
		IoCompleteApc(Apc);
	}
}

CSTATUS FcWriteParamRequest(
	fc_request_t *Request,
	const char *Buffer,
	size_t Size,
	OUT size_t *ActualSize)
{
	FC_REQUEST_IO_BLOCK block;
	CSTATUS status;

	block.Request = Request;
	block.Buffer = (char *)Buffer;
	block.Size = Size;
	
	status = IoInvokeApc(FcWriteParamRequestApc, &block);
	if (!SUCCESS(status))
		return status;

	if (block.Result < 0)
		return C_GENERIC_ERROR;

	*ActualSize = block.Result;
	return C_SUCCESS;
}
