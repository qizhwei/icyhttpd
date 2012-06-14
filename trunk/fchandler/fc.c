#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "fc.h"

static CSTATUS FcHandlerInit(void);

static CSTATUS FcHandlerCreate(
	OUT DM_HANDLER **Handler,
	DM_HANDLER_TYPE *Type,
	const char *Param);

static void FcHandlerInvoke(
	DM_HANDLER *Handler,
	DM_NODE *Node,
	const char *RelativePath,
	HTTP_CONNECTION *Connection,
	HTTP_REQUEST *Request,
	HTTP_RESPONSE *Response);

DM_HANDLER_TYPE FcHandlerType = {
	"fastcgi",
	FcHandlerInit,
	FcHandlerCreate,
	FcHandlerInvoke,
};

static CSTATUS FcHandlerInit(void)
{
	fc_startup();
	return C_SUCCESS;
}

static CSTATUS FcHandlerCreate(
	OUT DM_HANDLER **Handler,
	DM_HANDLER_TYPE *Type,
	const char *Param)
{
	FC_HANDLER *fc;

	fc = (FC_HANDLER *)malloc(sizeof(FC_HANDLER));
	if (fc == NULL)
		return C_BAD_ALLOC;

	fc->Handler.Type = &FcHandlerType;
	fc->Pool = fc_create_pool("D:\\php\\php-cgi.exe", 1000, 8, 5000, 499);
	if (fc->Pool == NULL) {
		free(fc);
		return C_GENERIC_ERROR;
	}

	*Handler = &fc->Handler;
	return C_SUCCESS;
}

static CSTATUS FcWriteParamWriter(
	BUFFERED_WRITER *Writer,
	const char *Name,
	const char *Value)
{
    size_t nameLen = strlen(Name);
    size_t valueLen = strlen(Value);
	char lenbuf[8];
	char *p = lenbuf;
	CSTATUS status;
	size_t actualSize;

    if (nameLen < 0x80) {
		*p++ = (char)nameLen;
    } else {
        *p++ = (char)(0x80 | (nameLen >> 24));
        *p++ = (char)(nameLen >> 16);
        *p++ = (char)(nameLen >> 8);
        *p++ = (char)nameLen;
    }
    if (valueLen < 0x80) {
        *p++ = (char)valueLen;
    } else {
        *p++ = (char)(0x80 | (valueLen >> 24));
        *p++ = (char)(valueLen >> 16);
        *p++ = (char)(valueLen >> 8);
        *p++ = (char)valueLen;
    }

	status = BufferedWriterWrite(Writer, lenbuf, p - lenbuf, &actualSize);
	if (!SUCCESS(status))
		return status;

	status = BufferedWriterWrite(Writer, Name, nameLen, &actualSize);
	if (!SUCCESS(status))
		return status;

	status = BufferedWriterWrite(Writer, Value, valueLen, &actualSize);
	if (!SUCCESS(status))
		return status;

	return C_SUCCESS;
}

static void FcHandlerInvoke(
	DM_HANDLER *Handler,
	DM_NODE *Node,
	const char *RelativePath,
	HTTP_CONNECTION *Connection,
	HTTP_REQUEST *Request,
	HTTP_RESPONSE *Response)
{
	FC_HANDLER *fc = (FC_HANDLER *)Handler;
	fc_request_t *req;
	BUFFERED_WRITER paramWriter;
	BUFFERED_READER dataReader;
	CSTATUS status;
	size_t actualSize;

	req = fc_create_request();
	if (req == NULL) {
		// TODO
	}

	status = FcBeginRequest(req, fc->Pool);
	if (!SUCCESS(status)) {
		// TODO
	}

	status = BufferedWriterInit(&paramWriter, FC_INITIAL_ALLOC_SIZE, FC_BLOCK_SIZE,
		(WRITE_FUNCTION *)FcWriteParamRequest, req);
	if (!SUCCESS(status)) {
		// TODO
	}

	// TODO
	status = FcWriteParamWriter(&paramWriter, "SCRIPT_FILENAME", "");
	status = FcWriteParamWriter(&paramWriter, "REQUEST_METHOD", "GET");
	status = BufferedWriterFlush(&paramWriter);
	BufferedWriterUninit(&paramWriter);
	status = FcWriteParamRequest(req, NULL, 0, &actualSize);

	status = BufferedReaderInit(&dataReader, FC_INITIAL_ALLOC_SIZE,
		(READ_FUNCTION *)FcReadRequest, req);

	while (1) {
		size_t dummy;
		status = BufferedReaderReadLine(&dataReader, FC_READ_SIZE, FC_READER_MAX_ALLOC_SIZE, 0, &dummy);
		if (!SUCCESS(status))
			break;
		LogPrintf("[fcpool output] %s\n", dataReader.Buffer.Data);
		BufferedReaderFlush(&dataReader);
	}

	BufferedReaderUninit(&dataReader);
}
