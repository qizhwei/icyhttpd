#include "server.h"
#include "shrimp.h"
#include "socket.h"
#include <stddef.h>

static void session_proc(void *param)
{
	socket_t *s = param;
	char buffer[4096];
	ssize_t size;

	// TODO: enhance this echo server
	while (1) {
		size = socket_read(s, buffer, sizeof(buffer));
		if (size == 0 || size == -1)
			break;
		if (socket_write(s, buffer, size) != size)
			break;
	}

	socket_destroy(s);
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
