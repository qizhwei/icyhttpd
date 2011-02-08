#include "server.h"
#include "process.h"
#include "socket.h"
#include "conn.h"

static void server_proc(void *param)
{
	socket_t *s = param;

	while (1) {
		socket_t *t = socket_accept(s);

		if (conn_create(t))
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

	if (process_create(&server_proc, s)) {
		socket_destroy(s);
		return -1;
	}

	return 0;
}
