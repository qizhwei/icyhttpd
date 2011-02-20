#include "conn.h"
#include "runtime.h"
#include "mem.h"
#include "str.h"
#include "process.h"
#include "socket.h"
#include "http.h"
#include "node.h"
#include "buf.h"
#include "endpoint.h"
#include "dict.h"
#include <stddef.h>
#include <stdio.h>

#define CONN_TIMEOUT (120000)

typedef struct conn {
	endpoint_t *endpoint;
	socket_t *socket;
	buf_t readbuf;
	buf_t writebuf;
	request_t request;
	response_t response;
} conn_t;

static void conn_destroy(conn_t *conn)
{
	socket_destroy(conn->socket);
	mem_free(conn);
}

static inline int write_header_proc(void *u, void *key, void *value)
{
	conn_t *conn = u;

	if (buf_put_str(&conn->writebuf, key)
		|| buf_put(&conn->writebuf, ": ")
		|| buf_put_str(&conn->writebuf, value)
		|| buf_put(&conn->writebuf, "\r\n"))
		return -1;

	return 0;
}

static int write_header(conn_t *conn)
{
	response_t *response = &conn->response;
	str_t *status;

	status = http_get_status(response->status);
	if (status == NULL) {
		// TODO: HTTP 500
		return -1;
	}

	if (buf_put(&conn->writebuf, "HTTP/1.1 ")
		|| buf_put_uint32(&conn->writebuf, response->status)
		|| buf_put(&conn->writebuf, " ")
		|| buf_put_str(&conn->writebuf, status)
		|| buf_put_crlf(&conn->writebuf)
		|| dict_walk(&response->headers, write_header_proc, conn)
		|| buf_put_crlf(&conn->writebuf))
		return -1;

	return 0;
}

static void conn_proc(void *param)
{
	conn_t *conn = param;
	request_t *request = &conn->request;
	response_t *response = &conn->response;
	char *line;
	node_t *node;
	str_t *ext;
	handler_t *handler;
	int keep_alive, chunked;
	time_t lt;
	str_t *date;

	printf("connection established\n");

	while (1) {
		do {
			if ((line = buf_gets(&conn->readbuf)) == NULL)
				goto bed;

			// ignore empty lines where a Request-Line is expected
		} while (line[0] == '\0');

		if (request_init(request, line))
			goto bed;

		printf("[request] method:%s req_uri:%s query_str:%s version:%d.%d\n",
			request->method->buffer, request->req_uri->buffer, request->query_str ? request->query_str->buffer : "(null)",
			request->ver.major, request->ver.minor);

		if (request->ver.major >= 1) {
			while (1) {
				if ((line = buf_gets(&conn->readbuf)) == NULL)
					goto bed0;
				if (*line == '\0')
					break;
				if (request_parse_header(request, line))
					goto bed0;
			}
		}

		if (request->ver.major >= 2) {
			// TODO: HTTP 505
			goto bed0;
		}

		if ((node = endpoint_get_node(conn->endpoint,
			request_get_header(request, str_literal("Host")))) == NULL)
			goto bed0;

		if ((ext = request_alloc_ext(request)) == NULL)
			goto bed0;
		if ((handler = node_get_handler(node, ext)) == NULL) {
			str_free(ext);
			goto bed0;
		}
		str_free(ext);

		// TODO: the handler must keep a reference to request / response
		// we must not free them too early (such as goto bed)
		// so what should we do?
		if (handler->handle_proc(handler, request, response))
			goto bed0;

		// TODO: create a process to read (at least ignore) when Content-Length
		// or Transfer-Encoding header field is included in the request's message-headers
		//
		// If a request contains a message-body and a Content-Length is not given,
		// the server SHOULD respond with 400 (bad request) if it cannot determine
		// the length of the message, or with 411 (length required) if it wishes to
		// insist on receiving a valid Content-Length.

		if (request->ver.major == 1 && request->ver.minor >= 1) {
			keep_alive = 1;
			chunked = 0; // TODO: change to 1 after chunk encoding is implemented
		} else {
			keep_alive = 0;
			chunked = 0;
		}

		// TODO: if `Connection: keep-alive' in request exist, keep_alive = 1
		// TODO: if `Connection: close' in request exist, keep_alive = 0

		if (response_get_header(response, str_literal("Content-Length")) != NULL)
			chunked = 0;
		else if (chunked == 0)
			keep_alive = 0;

		if (request->ver.major == 1) {
			// add `Server' field to response header
			response_add_header(response, str_literal("Server"), str_literal("icyhttpd/0.0"));

			// add `Connection' field to response header
			if (!keep_alive)
				response_add_header(response, str_literal("Connection"), str_literal("close"));
			else if (keep_alive && request->ver.minor == 0)
				response_add_header(response, str_literal("Connection"), str_literal("keep-alive"));

			// add `Transfer-Encoding' field to response header
			if (chunked)
				response_add_header(response, str_literal("Transfer-Encoding"), str_literal("chunked"));

			// add `Date' field to response header (TODO: is it right to put it here?)
			lt = time(NULL);
			date = http_alloc_date(gmtime(&lt));
			response_add_header(response, str_literal("Date"), date);
			str_free(date);

			if (write_header(conn))
				goto bed1;
		}

		if (!chunked) {
			if (buf_write_from_proc(&conn->writebuf, response->read_proc, response->object))
				goto bed1;
		} else {
			// TODO: chunked encoding
		}

		buf_flush(&conn->writebuf);
		if (!keep_alive)
			goto bed1;

		response_uninit(response);
		request_uninit(request);
	} // while (1)

bed1:
	response_uninit(response);
bed0:
	request_uninit(request);
bed:
	conn_destroy(conn);
	printf("connection broken\n");
}

static ssize_t read_proc(void *u, void *buffer, size_t size)
{
	process_timeout(CONN_TIMEOUT, (proc_t *)&socket_abort, u);
	return socket_read(u, buffer, size);
}

int conn_create(endpoint_t *e, socket_t *s)
{
	conn_t *conn = mem_alloc(sizeof(conn_t));
	conn->endpoint = e;
	conn->socket = s;
	buf_init(&conn->readbuf, read_proc, s);
	buf_init(&conn->writebuf, (io_proc_t *)&socket_write, s);

	if (process_create(&conn_proc, conn)) {
		mem_free(conn);
		return -1;
	}

	return 0;
}
