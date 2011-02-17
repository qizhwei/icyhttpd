#include "server.h"
#include "mem.h"
#include "str.h"
#include "dict.h"
#include "process.h"
#include "socket.h"
#include "message.h"
#include "node.h"
#include "buf.h"
#include <stddef.h>
#include <stdio.h>

#define CONN_TIMEOUT (120000)

struct endpoint {
	socket_t *socket;
	dict_t host_nodes;
	node_t *default_node;
};

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

	while (1) {
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
				if ((line = buf_gets(&conn->readbuf)) == NULL) {
					request_uninit(&request);
					goto bed;
				}

				if (*line == '\0')
					break;

				if (request_parse_header(&request, line)) {
					request_uninit(&request);
					goto bed;
				}
			}
		}

		dict_walk(&request.headers, debug_print_headers);

		// TODO
		if (request.method == str_literal("GET")) {
			buf_puts(&conn->writebuf, "HTTP/1.1 200 OK");
			buf_puts(&conn->writebuf, "");
			buf_puts(&conn->writebuf, "<h2>It works!</h2>");
			buf_put(&conn->writebuf, "<p>url: ");
			buf_put(&conn->writebuf, request.req_uri->buffer);
			buf_put(&conn->writebuf, "</p>");
			buf_flush(&conn->writebuf);
			request_uninit(&request);
			break;
		} else {

		}

		request_uninit(&request);
	}

bed:
	printf("connection broken\n");
	conn_destroy(conn);
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

static void server_proc(void *param)
{
	endpoint_t *e = param;

	while (1) {
		socket_t *t = socket_accept(e->socket);

		if (conn_create(e, t))
			socket_destroy(t);
	}
}

endpoint_t *endpoint_create(const char *ip, int port)
{
	endpoint_t *e = mem_alloc(sizeof(endpoint_t));
	if (e == NULL)
		return NULL;

	e->socket = socket_create();
	if (e->socket == NULL) {
		mem_free(e);
		return NULL;
	}

	dict_init(&e->host_nodes);
	e->default_node = NULL;

	if (socket_bind_ip(e->socket, ip, port)) {
		socket_destroy(e->socket);
		mem_free(e);
		return NULL;
	}

	if (process_create(&server_proc, e)) {
		socket_destroy(e->socket);
		mem_free(e);
		return NULL;
	}

	return e;
}
