#include <stdlib.h>
#include <string.h>
#include "daemon.h"

static CSTATUS StaticHandlerCreate(
	OUT DM_HANDLER **Handler,
	DM_HANDLER_TYPE *Type,
	const char *Param);

static void StaticHandlerInvoke(
	DM_HANDLER *Handler,
	DM_NODE *Node,
	const char *RelativePath,
	HTTP_CONNECTION *Connection,
	HTTP_REQUEST *Request,
	HTTP_RESPONSE *Response);

DM_HANDLER_TYPE StaticHandlerType = {
	"static",
	NULL,
	StaticHandlerCreate,
	StaticHandlerInvoke,
};

static DM_HANDLER StaticHandler = {
	&StaticHandlerType,
};

static CSTATUS StaticHandlerCreate(
	OUT DM_HANDLER **Handler,
	DM_HANDLER_TYPE *Type,
	const char *Param)
{
	*Handler = &StaticHandler;
	return C_SUCCESS;
}

static CSTATUS MyCreateFile(
	OUT IO_FILE **File,
	DM_NODE *Node,
	const char *RelativePath)
{
	CSTATUS status;
	size_t size;
	wchar_t *path;
	IO_FILE *file;

	status = Utf8ToUtf16GetSize(&size, RelativePath);
	if (!SUCCESS(status))
		return status;
	
	path = (wchar_t *)malloc((Node->DocumentRootSizeInChars + 1 + size) * 2);
	if (!path)
		return C_BAD_ALLOC;

	memcpy(path, Node->DocumentRoot, Node->DocumentRootSizeInChars * 2);
	path[Node->DocumentRootSizeInChars] = '\\';
	status = Utf8ToUtf16(&path[Node->DocumentRootSizeInChars + 1], size, RelativePath);
	if (!SUCCESS(status)) {
		free(path);
		return status;
	}

	status = IoCreateFile(&file, path);
	free(path);
	*File = file;
	return status;
}

// TODO: Support range request

static CSTATUS SendFileResponse(
	HTTP_RESPONSE *Response,
	IO_FILE *File)
{
	CSTATUS status;
	uint64_t fileSize;

	status = IoGetSizeFile(File, &fileSize);
	if (!SUCCESS(status))
		return status;

	status = HttpBeginHeaderResponse(Response, 200, "OK");
	if (!SUCCESS(status))
		return status;
	
	status = HttpPrintfResponse(Response, "Content-Length: %I64u\r\n", fileSize);
	if (!SUCCESS(status))
		return status;

	status = HttpEndHeaderResponse(Response, 1);
	if (!SUCCESS(status))
		return status;

	status = HttpTransmitFileResponse(Response, File, 0, fileSize);
	if (!SUCCESS(status))
		return status;

	return HttpEndResponse(Response);
}

static void StaticHandlerInvoke(
	DM_HANDLER *Handler,
	DM_NODE *Node,
	const char *RelativePath,
	HTTP_CONNECTION *Connection,
	HTTP_REQUEST *Request,
	HTTP_RESPONSE *Response)
{
	CSTATUS status;
	IO_FILE *file;

	status = MyCreateFile(&file, Node, RelativePath);
	if (!SUCCESS(status)) {
		HttpWriteErrorStatusResponse(Response, status);
		return;
	}

	status = SendFileResponse(Response, file);
	IoDestroyFile(file);

	if (!SUCCESS(status))
		HttpSetForceCloseResponse(Response);
}
