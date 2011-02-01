#include "server.h"
#include "shrimp.h"
#include "socket.h"
#include <stddef.h>
#include <string.h>

#define HTTP_MAX_LINE (8192)

typedef struct session {
	socket_t *socket;
	char buffer[HTTP_MAX_LINE];
	char *current, *last;
} session_t;

static void session_init(session_t *s, socket_t *socket)
{
	s->socket = socket;
	s->current = NULL;
	s->last = NULL;
}

static void session_destroy(session_t *s)
{
	socket_destroy(s->socket);
}

static char *session_read_line(session_t *s)
{
	char *buffer = s->buffer;
	char *current = s->current;
	char *last = s->last;
	size_t cur_size = last - current;
	char *next = memchr(current, '\n', cur_size);
	size_t read_size;

	if (next != NULL) {
		if (next[-1] == '\r')
			next[-1] = '\0';
		next[0] = '\0';
		++next;
		s->current = next;
		return current;
	}

	memmove(buffer, current, cur_size);
	current = buffer;
	last = current + cur_size;

	while (1) {
		if (cur_size == HTTP_MAX_LINE)
			return NULL;

		read_size = socket_read(s->socket, last, HTTP_MAX_LINE - cur_size);
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
			s->current = next;
			s->last = last;
			return current;
		}
	}
}

static void session_proc(void *param)
{
	session_t s;
	char *line;

	session_init(&s, param);

	// TODO: session_read_line(&s)

	session_destroy(&s);
}

static void server_proc(void *param)
{
	socket_t *s = param;

	while (1) {
		socket_t *t = socket_accept(s);

		if (shrimp_create(&session_proc, t) == NULL)
			socket_destroy(t);
	}
}

int server_add_ip(const char *ip, int port)
{
	socket_t *s = socket_create();
	if (s == NULL)
		return -1;

	if (socket_bind_ip(s, ip, port)) {
		socket_destroy(s);
		return -1;
	}

	if (shrimp_create(&server_proc, s) == NULL) {
		socket_destroy(s);
		return -1;
	}

	return 0;
}
