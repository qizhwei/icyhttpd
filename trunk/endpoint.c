#include "endpoint.h"
#include "socket.h"
#include "dict.h"
#include "node.h"
#include "conn.h"

struct endpoint {
	socket_t *socket;
	dict_t host_nodes;
	node_t *default_node;
};

static void accept_proc(void *param)
{
	endpoint_t *e = param;

	while (1) {
		socket_t *t = socket_accept(e->socket);

		if (conn_create(e, t))
			socket_destroy(t);
	}
}

endpoint_t *endpoint_create(const char *ip, int port, node_t *default_node)
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
	e->default_node = default_node;

	if (socket_bind_ip(e->socket, ip, port)) {
		socket_destroy(e->socket);
		mem_free(e);
		return NULL;
	}

	if (process_create(&accept_proc, e)) {
		socket_destroy(e->socket);
		mem_free(e);
		return NULL;
	}

	return e;
}

node_t *endpoint_get_node(endpoint_t *endpoint, str_t *host)
{
	node_t *node = endpoint->default_node;
	void **value;
	handler_t *handler;

	if (host != NULL) {
		host = str_lower(host);
		if (host == NULL)
			return NULL;

		value = dict_query_ptr(&endpoint->host_nodes, host, 0);
		str_free(host);
		if (value != NULL)
			node = *value;
	}

	return node;
}
