#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "protocol.h"

CSTATUS HttpInitResponse(
	OUT HTTP_RESPONSE *Response,
	IO_CLIENT *IoClient)
{
	CSTATUS status;

	status = BufferedWriterInit(&Response->Writer,
		BUFFER_INITIAL_ALLOC_SIZE, BUFFER_BLOCK_SIZE,
		(WRITE_FUNCTION *)IoWriteClient, IoClient);
	if (!SUCCESS(status))
		return status;

	Response->AssumeKeepAlive = 0;
	Response->Entity = 0;
	Response->KeepAlive = 0;
	Response->Chunked = 0;

	return C_SUCCESS;
}

void HttpUninitResponse(
	HTTP_RESPONSE *Response)
{
	BufferedWriterUninit(&Response->Writer);
}

void HttpAttachRequestResponse(
	HTTP_RESPONSE *Response,
	HTTP_REQUEST *Request)
{
	Response->KeepAlive = Request->KeepAlive;

	if (Request->MajorVersion == 1) {
		if (Request->MinorVersion >= 1) {
			Response->AssumeKeepAlive = 1;
			Response->Chunked = 1;
		}
	} else if (Request->MajorVersion == 0) {
		Response->Entity = 1;
	}
}

CSTATUS HttpVPrintfResponse(
	HTTP_RESPONSE *Response,
	const char *Format,
	va_list ArgList)
{
	return BufferedWriterVPrintf(&Response->Writer, Format, ArgList);
}

CSTATUS HttpPrintfResponse(
	HTTP_RESPONSE *Response,
	const char *Format, ...)
{
	CSTATUS status;
	va_list args;

	va_start(args, Format);
	status = HttpVPrintfResponse(Response, Format, args);
	va_end(args);

	return status;
}

CSTATUS HttpBeginHeaderResponse(
	HTTP_RESPONSE *Response,
	int StatusCode,
	const char *ReasonPhrase)
{
	if (Response->Entity)
		return C_SUCCESS;

	return HttpPrintfResponse(Response, "HTTP/1.1 %d %s\r\n",
		StatusCode, ReasonPhrase);
}

CSTATUS HttpEndHeaderResponse(
	HTTP_RESPONSE *Response,
	int LengthProvided)
{
	CSTATUS status;

	if (Response->Entity)
		return C_SUCCESS;

	// Server header
	status = HttpPrintfResponse(Response,
		"Server: icyhttpd/0.0\r\n");
	if (!SUCCESS(status))
		return status;

	if (LengthProvided)
		Response->Chunked = 0;
	else if (!Response->Chunked)
		Response->KeepAlive = 0;

	// Transfer header
	if (Response->Chunked) {
		status = HttpPrintfResponse(Response,
			"Transfer-Encoding: chunked\r\n");
		if (!SUCCESS(status))
			return status;
	}

	// Connection header
	if (!Response->KeepAlive) {
		status = HttpPrintfResponse(Response,
			"Connection: close\r\n");
		if (!SUCCESS(status))
			return status;
	} else if (!Response->AssumeKeepAlive) {
		status = HttpPrintfResponse(Response,
			"Connection: keep-alive\r\n");
		if (!SUCCESS(status))
			return status;
	}

	return HttpPrintfResponse(Response, "\r\n");
}

CSTATUS HttpWriteResponse(
	HTTP_RESPONSE *Response,
	const char *Buffer,
	size_t Size,
	OUT size_t *ActualSize)
{
	CSTATUS status;
	size_t actualSize;

	if (Response->Chunked) {
		status = BufferedWriterPrintf(&Response->Writer, "%x\r\n", (unsigned int)Size);
		if (!SUCCESS(status))
			return status;

		status = BufferedWriterWrite(&Response->Writer, Buffer, Size, &actualSize);
		if (!SUCCESS(status))
			return status;

		status = BufferedWriterPrintf(&Response->Writer, "\r\n");
		if (!SUCCESS(status))
			return status;

		*ActualSize = Size;
		return C_SUCCESS;
	} else {
		return BufferedWriterWrite(&Response->Writer, Buffer, Size, ActualSize);
	}
}

CSTATUS HttpTransmitFileResponse(
	HTTP_RESPONSE *Response,
	IO_FILE *File,
	uint64_t Offset,
	uint64_t Length)
{
	CSTATUS status;

	assert(!Response->Chunked);

	status = BufferedWriterFlush(&Response->Writer);
	if (!SUCCESS(status))
		return status;

	return IoTransmitFileClient((IO_CLIENT *)Response->Writer.Context, File, Offset, Length);
}

CSTATUS HttpEndResponse(
	HTTP_RESPONSE *Response)
{
	CSTATUS status;

	if (Response->Chunked) {
		status = BufferedWriterPrintf(&Response->Writer, "0\r\n\r\n");
		if (!SUCCESS(status))
			return status;
	}

	return C_SUCCESS;
}

void HttpSetForceCloseResponse(
	HTTP_RESPONSE *Response)
{
	Response->KeepAlive = 0;
}

CSTATUS HttpVPrintfErrorResponse(
	HTTP_RESPONSE *Response,
	int StatusCode,
	const char *ReasonPhrase,
	const char *Format,
	va_list ArgList)
{
	CSTATUS status;
	BUFFERED_WRITER writer;

	status = HttpBeginHeaderResponse(Response, StatusCode, ReasonPhrase);
	if (!SUCCESS(status))
		return status;
	status = HttpPrintfResponse(Response, "Content-Type: text/html\r\n");
	if (!SUCCESS(status))
		return status;
	status = HttpEndHeaderResponse(Response, 0);
	if (!SUCCESS(status))
		return status;

	status = BufferedWriterInit(&writer,
		BUFFER_INITIAL_ALLOC_SIZE, BUFFER_BLOCK_SIZE,
		(WRITE_FUNCTION *)HttpWriteResponse, Response);
	if (!SUCCESS(status))
		return status;

	status = BufferedWriterPrintf(&writer,
		"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
		"<html><head>\r\n"
		"<title>%d %s</title>\r\n"
		"</head><body>\r\n"
		"<h1>%s</h1>\r\n", StatusCode, ReasonPhrase, ReasonPhrase);
	if (!SUCCESS(status)) {
		BufferedWriterUninit(&writer);
		return status;
	}

	status = BufferedWriterVPrintf(&writer, Format, ArgList);
	if (!SUCCESS(status)) {
		BufferedWriterUninit(&writer);
		return status;
	}

	status = BufferedWriterPrintf(&writer,
		"<hr>\r\n"
		"<address>icyhttpd/0.0</address>\r\n"
		"</body></html>");
	if (!SUCCESS(status)) {
		BufferedWriterUninit(&writer);
		return status;
	}

	BufferedWriterUninit(&writer);
	return HttpEndResponse(Response);
}

CSTATUS HttpPrintfErrorResponse(
	HTTP_RESPONSE *Response,
	int StatusCode,
	const char *ReasonPhrase,
	const char *Format, ...)
{
	CSTATUS status;
	va_list args;

	va_start(args, Format);
	status = HttpVPrintfErrorResponse(Response,
		StatusCode, ReasonPhrase, Format, args);
	va_end(args);

	return status;
}

void HttpWriteErrorStatusResponse(
	HTTP_RESPONSE *Response,
	CSTATUS Status)
{
	switch (Status) {
	case C_ACCESS_DENIED:
		HttpPrintfErrorResponse(Response, 403, "Forbidden",
			"<p>You don't have permission to access the requested URL.</p>");
		break;
	case C_NOT_FOUND:
		HttpPrintfErrorResponse(Response, 404, "Not Found",
			"<p>The requested URL was not found on this server.</p>");
		break;
	case C_NOT_IMPLEMENTED:
		HttpPrintfErrorResponse(Response, 500, "Internal Server Error",
			"<p>The requested function is not implemented.</p>");
		break;
	default:
		HttpPrintfErrorResponse(Response, 500, "Internal Server Error",
			"<p>The server has encountered an internal error.</p>");
	}
}
