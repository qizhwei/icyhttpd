#include "socket.h"
#include "win32.h"
#include "shrimp.h"
#include "list.h"
#include <stdlib.h>

typedef struct async_accept {
	socket_t *socket;
	handle_t shrimp;
	SOCKET accept_socket;
} async_accept_t;

typedef struct async_io {
	OVERLAPPED overlapped;
	handle_t shrimp;
	ssize_t result;
} async_io_t;

struct socket {
	SOCKET os_socket;
	async_accept_t *async_accept;
};

typedef struct sockaddr_in ip_endpoint_t;

int socket_init(void)
{
	WSADATA data;

	if (WSAStartup(MAKEWORD(2, 2), &data))
		return -1;

	return 0;
}

static socket_t *socket_create_from(SOCKET os_socket)
{
	socket_t *s = malloc(sizeof(socket_t));
	if (s == NULL)
		return NULL;

	s->os_socket = os_socket;
	s->async_accept = NULL;
	return s;
}

socket_t *socket_create(void)
{
	SOCKET os_socket;
	socket_t *s;

	os_socket = socket(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP);
	if (os_socket == INVALID_SOCKET)
		return NULL;

	s = socket_create_from(os_socket);
	if (s == NULL) {
		closesocket(os_socket);
		return NULL;
	}

	return s;
}

void socket_destroy(socket_t *s)
{
	closesocket(s->os_socket);
	free(s);
}

static void event_proc(void *param)
{
	socket_t *s = param;
	async_accept_t *async = s->async_accept;
	WSANETWORKEVENTS events;

	if (async == NULL)
		return;

	if (WSAEnumNetworkEvents(s->os_socket, NULL, &events))
		return;

	if (events.lNetworkEvents & FD_ACCEPT) {
		async->accept_socket = accept(s->os_socket, NULL, NULL);
		shrimp_switch(async->shrimp);
	}
}

int socket_bind_ip(socket_t *s, const char *ip, int port)
{
	ip_endpoint_t ep;
	HANDLE event;

	ep.sin_family = AF_INET;
	ep.sin_addr.s_addr = inet_addr(ip);
	ep.sin_port = htons(port);

	if (bind(s->os_socket, (SOCKADDR *)&ep, sizeof(ep)))
		return -1;

	if (listen(s->os_socket, SOMAXCONN))
		return -1;

	event = shrimp_share_event(&event_proc, s);
	if (event == NULL)
		return -1;

	if (WSAEventSelect(s->os_socket, event, FD_ACCEPT))
		return -1;

	return 0;
}

socket_t *socket_accept(socket_t *s)
{
	SOCKET as = accept(s->os_socket, NULL, NULL);
	socket_t *result;
	async_accept_t async;

	if (as == INVALID_SOCKET) {
		if (GetLastError() != WSAEWOULDBLOCK)
			return NULL;

		// blocking operation, event_proc will be called later
		async.shrimp = shrimp_current();
		s->async_accept = &async;
		shrimp_block();
		s->async_accept = NULL;
		as = async.accept_socket;
	}

	if (as == INVALID_SOCKET)
		return NULL;

	result = socket_create_from(as);
	if (result == NULL) {
		closesocket(as);
		return NULL;
	}

	return result;
}

static void CALLBACK io_proc(DWORD error, DWORD size, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	async_io_t *async = CONTAINER_OF(overlapped, async_io_t, overlapped);

	if (error)
		async->result = -1;
	else
		async->result = size;

	shrimp_switch(async->shrimp);
}

ssize_t socket_read(socket_t *s, char *buffer, size_t size)
{
	WSABUF buf = {size, buffer};
	DWORD flags = 0;
	async_io_t async;

	memset(&async.overlapped, 0, sizeof(WSAOVERLAPPED));
	if (WSARecv(s->os_socket, &buf, 1, NULL, &flags, &async.overlapped, &io_proc)) {
		if (GetLastError() != WSA_IO_PENDING)
			return -1;
	}

	async.shrimp = shrimp_current();
	shrimp_block();
	return async.result;
}

ssize_t socket_write(socket_t *s, char *buffer, size_t size)
{
	WSABUF buf = {size, buffer};
	async_io_t async;

	memset(&async.overlapped, 0, sizeof(WSAOVERLAPPED));
	if (WSASend(s->os_socket, &buf, 1, NULL, 0, &async.overlapped, &io_proc)) {
		if (GetLastError() != WSA_IO_PENDING)
			return -1;
	}

	async.shrimp = shrimp_current();
	shrimp_block();
	return async.result;
}
