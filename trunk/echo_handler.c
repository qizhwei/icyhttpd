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
	char buffer[4096];
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

static void session_proc(void *u)
{
	session_t *session = u;
	buf_t writebuf;

	buf_init(&writebuf, (io_proc_t *)&fifo_write, &session->fifo);
	buf_put(&writebuf, session->buffer)
		|| buf_flush(&writebuf);
	fifo_write(&session->fifo, NULL, 0);
}

static int handle_proc(handler_t *handler, request_t *request, response_t *response)
{
	session_t *session;
	str_t *content_length;

	if ((session = mem_alloc(sizeof(session_t))) == NULL)
		return -1;

	session->buffer[0] = '\0';
	strncat(session->buffer, "Request URL: ", sizeof(session->buffer));
	strncat(session->buffer, request->req_uri->buffer, sizeof(session->buffer));
	fifo_init(&session->fifo);

	response_init(response, 200, &read_proc, &write_proc, &close_proc, session);
	if ((content_length = str_uint32(strlen(session->buffer))) == NULL
		|| response_add_header(response, str_literal("Content-Length"), content_length)
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
