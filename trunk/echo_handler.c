#include "echo_handler.h"
#include "dict.h"
#include "mem.h"
#include "fifo.h"
#include "buf.h"
#include <stddef.h>

typedef struct echo_handler {
	handler_t handler;
} echo_handler_t;

typedef struct session {
	request_t *request;
	fifo_t fifo;
} session_t;

static echo_handler_t handler;
handler_t *echo_handler;

static size_t read_proc(void *u, void *buffer, size_t size)
{
	session_t *session = u;
	return fifo_read(&session->fifo, buffer, size);
}

static size_t write_proc(void *u, void *buffer, size_t size)
{
	// write failed, insisting that we do not want the request entity
	return 0;
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
	buf_put(&writebuf, "Request URL: ") ||
	buf_puts(&writebuf, session->request->req_uri->buffer) ||
	buf_flush(&writebuf);

	fifo_close(&session->fifo);
}

static int handle_proc(handler_t *handler, request_t *request, response_t *response)
{
	session_t *session;

	if ((session = mem_alloc(sizeof(session_t))) == NULL)
		return -1;

	session->request = request;
	fifo_init(&session->fifo);

	if (process_create(&session_proc, session)) {
		mem_free(session);
		return -1;
	}

	response->ver.major = 1;
	response->ver.minor = 1;
	response->status = 200;
	dict_init(&response->headers);

	response->read_proc = &read_proc;
	response->write_proc = &write_proc;
	response->close_proc = &close_proc;
	response->object = session;

	return 0;
}

void echo_handler_init(void)
{
	handler.handler.handle_proc = handle_proc;
	echo_handler = &handler.handler;
}
