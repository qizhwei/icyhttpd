#include "conn.h"
#include "mem.h"
#include "str.h"
#include "process.h"
#include "socket.h"
#include "message.h"
#include "node.h"
#include "buf.h"
#include "endpoint.h"
#include <stddef.h>
#include <stdio.h>
#include "dict.h"

#define CONN_TIMEOUT (120000)

typedef struct conn {
	endpoint_t *endpoint;
	socket_t *socket;
	buf_t readbuf;
	buf_t writebuf;
} conn_t;

static void conn_destroy(conn_t *conn)
{
	socket_destroy(conn->socket);
	mem_free(conn);
}

static inline void debug_print_headers(void *key, void *value)
{
	printf("[header] %s: %s\n", ((str_t *)key)->buffer, ((str_t *)value)->buffer);
}

static void conn_proc(void *param)
{
	conn_t *conn = param;
	char *line;
	request_t request;
	response_t response;
	node_t *node;
	str_t *ext;
	handler_t *handler;

	do {
		do {
			if ((line = buf_gets(&conn->readbuf)) == NULL)
				goto bed;

			// ignore empty lines where a Request-Line is expected
		} while (line[0] == '\0');

		if (request_init(&request, line))
			goto bed;

		printf("[request] method:%s req_uri:%s query_str:%s version:%d.%d\n",
			request.method->buffer, request.req_uri->buffer, request.query_str ? request.query_str->buffer : "(null)",
			request.ver.major, request.ver.minor);

		if (request.ver.major >= 1) {
			while (1) {
				if ((line = buf_gets(&conn->readbuf)) == NULL)
					goto bed0;
				if (*line == '\0')
					break;
				if (request_parse_header(&request, line))
					goto bed0;
			}
		}

		dict_walk(&request.headers, debug_print_headers);

		if ((node = endpoint_get_node(conn->endpoint,
			request_get_header(&request, str_literal("Host")))) == NULL)
			goto bed0;

		if ((ext = request_alloc_ext(&request)) == NULL)
			goto bed0;
		if ((handler = node_get_handler(node, ext)) == NULL) {
			str_free(ext);
			goto bed0;
		}
		str_free(ext);

		// TODO: do we need to initialize response object first?

		if (handler->handle_proc(handler, &request, &response))
			goto bed0;

		// TODO: response->read_proc / response->write_proc / response->close_proc

		request_uninit(&request);
	} while (1); // TODO: !response.must_close, etc.

bed0:
	request_uninit(&request);
bed:
	conn_destroy(conn);
	printf("connection broken\n");
}

static size_t read_proc(void *u, void *buffer, size_t size)
{
	if (process_timeout(CONN_TIMEOUT, (proc_t *)&socket_abort, u))
		return 0;

	return socket_read(u, buffer, size);
}

int conn_create(endpoint_t *e, socket_t *s)
{
	conn_t *conn = mem_alloc(sizeof(conn_t));
	if (conn == NULL)
		return -1;

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
