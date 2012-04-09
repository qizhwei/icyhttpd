#ifndef HTTPD_PROTOCOL_H
#define HTTPD_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>
#include <misc.h>
#include <cstatus.h>
#include <httpd_util.h>
#include <httpd_io.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _HTTP_ENDPOINT {
	IO_LISTENER *Listener;
	IO_THREAD *Thread;
} HTTP_ENDPOINT;

typedef struct _HTTP_CONNECTION {
	HTTP_ENDPOINT *Endpoint;
	IO_CLIENT *Client;
} HTTP_CONNECTION;

typedef struct _HTTP_HEADER {
	size_t Name;
	size_t Value;
} HTTP_HEADER;

#define HTTP_MAX_HEADERS (256)

typedef struct _HTTP_REQUEST {
	BUFFERED_READER *Reader;
	size_t Method;
	size_t URI;
	size_t Extension; // optional
	size_t QueryString; // optional
	uint16_t MajorVersion;
	uint16_t MinorVersion;
	size_t Host; // optional
	int Chunked;
	int KeepAlive;
	uint64_t RemainingLength;
	size_t HeaderCount;
	HTTP_HEADER UnsortedHeaders[HTTP_MAX_HEADERS];
	HTTP_HEADER SortedHeaders[HTTP_MAX_HEADERS];
} HTTP_REQUEST;

typedef struct _HTTP_RESPONSE {
	BUFFERED_WRITER Writer;
	int AssumeKeepAlive;
	int Entity;
	int KeepAlive;
	int Chunked;
} HTTP_RESPONSE;

extern CSTATUS HttpInitEndpoint(
	OUT HTTP_ENDPOINT *Endpoint,
	const char *IPAddress,
	int Port);

extern void HttpMain(
	HTTP_CONNECTION *Connection,
	HTTP_REQUEST *Request,
	HTTP_RESPONSE *Response);

extern void HttpHeaderByIndexRequest(
	HTTP_REQUEST *Request,
	size_t Index,
	OUT char **HeaderName,
	OUT char **HeaderValue);

extern char *HttpHeaderByNameRequest(
	HTTP_REQUEST *Request,
	const char *HeaderName);

extern char *HttpGetMethodRequest(
	HTTP_REQUEST *Request);

extern char *HttpGetURIRequest(
	HTTP_REQUEST *Request);

extern char *HttpGetExtensionRequest(
	HTTP_REQUEST *Request);

extern char *HttpGetQueryStringRequest(
	HTTP_REQUEST *Request);

extern char *HttpGetHostRequest(
	HTTP_REQUEST *Request);

extern CSTATUS HttpReadRequest(
	HTTP_REQUEST *Request,
	char *Buffer,
	size_t Size,
	OUT size_t *ActualSize);

extern CSTATUS HttpVPrintfResponse(
	HTTP_RESPONSE *Response,
	const char *Format,
	va_list ArgList);

extern CSTATUS HttpPrintfResponse(
	HTTP_RESPONSE *Response,
	const char *Format, ...);

extern CSTATUS HttpBeginHeaderResponse(
	HTTP_RESPONSE *Response,
	int StatusCode,
	const char *ReasonPhrase);

extern CSTATUS HttpEndHeaderResponse(
	HTTP_RESPONSE *Response,
	int LengthProvided);

extern CSTATUS HttpWriteResponse(
	HTTP_RESPONSE *Response,
	const char *Buffer,
	size_t Size,
	OUT size_t *ActualSize);

extern CSTATUS HttpTransmitFileResponse(
	HTTP_RESPONSE *Response,
	IO_FILE *File,
	uint64_t Offset,
	uint64_t Length);

extern CSTATUS HttpEndResponse(
	HTTP_RESPONSE *Response);

extern void HttpSetForceCloseResponse(
	HTTP_RESPONSE *Response);

extern CSTATUS HttpVPrintfErrorResponse(
	HTTP_RESPONSE *Response,
	int StatusCode,
	const char *ReasonPhrase,
	const char *Format,
	va_list ArgList);

extern CSTATUS HttpPrintfErrorResponse(
	HTTP_RESPONSE *Response,
	int StatusCode,
	const char *ReasonPhrase,
	const char *Format, ...);

extern void HttpWriteErrorStatusResponse(
	HTTP_RESPONSE *Response,
	CSTATUS Status);

#ifdef __cplusplus
}
#endif

#endif
