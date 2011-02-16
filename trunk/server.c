#include "server.h"
#include "mem.h"
#include "str.h"
#include "dict.h"
#include "process.h"
#include "socket.h"
#include "message.h"
#include "node.h"
#include <stddef.h>
#include <string.h>
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
	char *current, *last;
	char buffer[HTTP_MAX_LINE];
} conn_t;

static char *conn_gets(conn_t *conn)
{
	char *buffer = conn->buffer;
	char *current = conn->current;
	char *last = conn->last;
	size_t cur_size = last - current;
	char *next = memchr(current, '\n', cur_size);
	size_t read_size;

	if (next != NULL) {
		if (next[-1] == '\r')
			next[-1] = '\0';
		next[0] = '\0';
		++next;
		conn->current = next;
		return current;
	}

	memmove(buffer, current, cur_size);
	current = buffer;
	last = current + cur_size;

	while (1) {
		if (cur_size == HTTP_MAX_LINE)
			return NULL;

		if (process_timeout(CONN_TIMEOUT, (proc_t *)&socket_abort, conn->socket))
			return NULL;

		read_size = socket_read(conn->socket, last, HTTP_MAX_LINE - cur_size);
		if (read_size == 0)
			return NULL;

		cur_size += read_size;
		next = memchr(last, '\n', cur_size);
		last += read_size;

		if (next != NULL) {
			if (next[-1] == '\r')
				next[-1] = '\0';
			next[0] = '\0';
			++next;
			conn->current = next;
			conn->last = last;
			return current;
		}
	}
}

static size_t conn_read(conn_t *conn, char *buffer, size_t size)
{
	// TODO: read conn->buffer first!
	return socket_read(conn->socket, buffer, size);
}

static size_t conn_write(conn_t *conn, char *buffer, size_t size)
{
	return socket_write(conn->socket, buffer, size);
}

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
			if ((line = conn_gets(conn)) == NULL)
				goto bed;

			// ignore empty lines where a Request-Line is expected
		} while (line[0] == '\0');

		if (request_init(&request, line))
			goto bed;

		printf("[request] method:%s url:%s version:%d.%d\n",
			request.method->buffer, request.req_uri->buffer,
			request.ver.major, request.ver.minor);

		// TODO: do HTTP/0.9 client send headers?

		while (1) {
			if ((line = conn_gets(conn)) == NULL) {
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

		dict_walk(&request.headers, debug_print_headers);
	}

bed:
	printf("connection broken\n");
	conn_destroy(conn);
}

int conn_create(endpoint_t *e, socket_t *s)
{
	conn_t *conn = mem_alloc(sizeof(conn_t));
	if (conn == NULL)
		return -1;

	conn->endpoint = e;
	conn->socket = s;
	conn->current = NULL;
	conn->last = NULL;

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
