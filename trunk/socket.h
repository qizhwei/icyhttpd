#ifndef _SOCKET_H
#define _SOCKET_H

#include "runtime.h"
#include <stddef.h>

typedef struct socket socket_t;

extern MAYFAIL(-1) int socket_init(void);
extern MAYFAIL(NULL) socket_t *socket_create(void);
extern void socket_destroy(socket_t *s);
extern void socket_abort(socket_t *s);
extern MAYFAIL(-1) int socket_bind_ip(socket_t *s, const char *ip, int port);
extern MAYFAIL(NULL) socket_t *socket_accept(socket_t *s);
extern MAYFAIL(-1) ssize_t socket_read(socket_t *s, void *buffer, size_t size);
extern MAYFAIL(-1) ssize_t socket_write(socket_t *s, void *buffer, size_t size);

#endif
