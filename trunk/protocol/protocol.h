#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <cstatus.h>
#include <httpd_protocol.h>
#include <httpd_util.h>
#include <httpd_io.h>

#define BUFFER_INITIAL_ALLOC_SIZE (8192)
#define BUFFER_BLOCK_SIZE (4096)

extern CSTATUS HttpInitRequest(
	OUT HTTP_REQUEST *Request,
	BUFFERED_READER *Reader);

extern void HttpUninitRequest(
	HTTP_REQUEST *Request);

extern CSTATUS HttpInitResponse(
	OUT HTTP_RESPONSE *Response,
	IO_CLIENT *IoClient);

extern void HttpAttachRequestResponse(
	HTTP_RESPONSE *Response,
	HTTP_REQUEST *Request);

extern void HttpUninitResponse(
	HTTP_RESPONSE *Response);

#endif
