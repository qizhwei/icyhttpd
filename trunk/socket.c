#include "socket.h"
#include "mem.h"
#include "win32.h"
#include "process.h"
#include "list.h"
#include <stdlib.h>

typedef struct async_accept {
	async_t async;
	socket_t *socket;
	SOCKET accept_socket;
} async_accept_t;

typedef struct async_io {
	async_t async;
	OVERLAPPED overlapped;
	size_t result;
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
	socket_t *s = mem_alloc(sizeof(socket_t));
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
	mem_free(s);
}

void socket_abort(socket_t *s)
{
	closesocket(s->os_socket);
	s->os_socket = INVALID_SOCKET;
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
		process_unblock(&async->async);
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

	event = process_share_event(&event_proc, s);
	if (event == NULL)
		return -1;

	if (WSAEventSelect(s->os_socket, event, FD_ACCEPT))
		return -1;

	return 0;
}

socket_t *socket_accept(socket_t *s)
{
	async_accept_t async;
	SOCKET as;
	socket_t *result;

	as = accept(s->os_socket, NULL, NULL);
	if (as == INVALID_SOCKET) {
		if (GetLastError() != WSAEWOULDBLOCK)
			return NULL;

		// blocking operation, event_proc will be called later
		s->async_accept = &async;
		process_block(&async.async);
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
	process_t *process = async->async.process;

	if (error)
		async->result = 0;
	else
		async->result = size;

	if (process)
		process_unblock(&async->async);
}

size_t socket_read(socket_t *s, void *buffer, size_t size)
{
	WSABUF buf = {size, buffer};
	DWORD flags = 0;
	async_io_t async;
	size_t result;

	memset(&async.overlapped, 0, sizeof(WSAOVERLAPPED));
	if (WSARecv(s->os_socket, &buf, 1, NULL, &flags, &async.overlapped, &io_proc)) {
		if (GetLastError() != WSA_IO_PENDING)
			return 0;
	}

	process_block(&async.async);
	return async.result;
}

size_t socket_write(socket_t *s, void *buffer, size_t size)
{
	WSABUF buf = {size, buffer};
	async_io_t async;
	size_t result;

	memset(&async.overlapped, 0, sizeof(WSAOVERLAPPED));
	if (WSASend(s->os_socket, &buf, 1, NULL, 0, &async.overlapped, &io_proc)) {
		if (GetLastError() != WSA_IO_PENDING)
			return 0;
	}

	process_block(&async.async);
	return async.result;
}
