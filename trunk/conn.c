#include "conn.h"
#include "socket.h"
#include "process.h"
#include <stddef.h>
#include <string.h>

#define HTTP_MAX_LINE (8192)
#define GETS_TIMEOUT (120000)

typedef struct conn {
	socket_t *socket;
	char buffer[HTTP_MAX_LINE];
	char *current, *last;
} conn_t;

static void conn_init(conn_t *conn, socket_t *s)
{
	conn->socket = s;
	conn->current = NULL;
	conn->last = NULL;
}

static void conn_destroy(conn_t *conn)
{
	socket_destroy(conn->socket);
}

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

		if (process_timeout(GETS_TIMEOUT, (proc_t *)&socket_abort, conn->socket))
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
	return socket_read(conn->socket, buffer, size);
}

static size_t conn_write(conn_t *conn, char *buffer, size_t size)
{
	return socket_write(conn->socket, buffer, size);
}

static void conn_proc(void *param)
{
	conn_t conn;
	char *line;

	conn_init(&conn, param);

	while (1) {
		// TODO: extend this `line echo server'

		if ((line = conn_gets(&conn)) == NULL)
			break;
		if (conn_write(&conn, line, strlen(line)) == 0)
			break;
		if (conn_write(&conn, "\r\n", 2) == 0)
			break;
	}

	conn_destroy(&conn);
}

int conn_create(socket_t *s)
{
	if (process_create(&conn_proc, s))
		return -1;

	return 0;
}
