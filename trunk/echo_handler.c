#include "echo_handler.h"
#include "dict.h"
#include "str.h"
#include "mem.h"
#include "fifo.h"
#include "buf.h"
#include "proc.h"
#include <stddef.h>
#include <string.h>

typedef struct echo_handler {
	handler_t handler;
} echo_handler_t;

typedef struct session {
	fifo_t fifo;
	request_t *request;
} session_t;

static echo_handler_t handler;
handler_t *echo_handler;

static ssize_t read_proc(void *u, void *buffer, size_t size)
{
	session_t *session = u;
	return fifo_read(&session->fifo, buffer, size);
}

static ssize_t write_proc(void *u, void *buffer, size_t size)
{
	// write failed, insisting that we do not want the request entity
	return -1;
}

static void close_proc(void *u)
{
	mem_free(u);
}

static int write_headers(void *u, void *key, void *value)
{
	if (buf_put_str(u, str_literal("\r\n"))
		|| buf_put_str(u, key)
		|| buf_put_str(u, str_literal("="))
		|| buf_put_str(u, value))
		return -1;
	return 0;
}

static void session_proc(void *u)
{
	session_t *session = u;
	request_t *request = session->request;
	buf_t wb;

	buf_init(&wb, (io_proc_t *)&fifo_write, &session->fifo);
	buf_put_str(&wb, str_literal("icyhttpd echo handler\r\n\r\n[request]\r\nmethod="))
	|| buf_put_str(&wb, request->method)
	|| buf_put_str(&wb, str_literal("\r\nreq_uri="))
	|| buf_put_str(&wb, request->req_uri)
	|| buf_put_str(&wb, str_literal("\r\nquery_str="))
	|| buf_put_str(&wb, request->query_str != NULL ? request->query_str : str_literal("(null)"))
	|| buf_put_str(&wb, str_literal("\r\n\r\n[Headers]"))
	|| dict_walk(&request->headers, write_headers, &wb)
	|| buf_flush(&wb);

	fifo_write(&session->fifo, NULL, 0);
}

static int handle_proc(handler_t *handler, request_t *request, response_t *response)
{
	session_t *session;

	if ((session = mem_alloc(sizeof(session_t))) == NULL)
		return -1;

	fifo_init(&session->fifo);
	session->request = request;

	response_init(response, 200, &read_proc, &write_proc, &close_proc, session);
	if (response_add_header(response, str_literal("Content-Type"), str_literal("text/plain"))
		|| process_create(&session_proc, session))
	{
		response_uninit(response);
		return -1;
	}

	return 0;
}

void echo_handler_init(void)
{
	handler.handler.handle_proc = handle_proc;
	echo_handler = &handler.handler;
}
