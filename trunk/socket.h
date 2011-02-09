#ifndef _SOCKET_H
#define _SOCKET_H

#include <stddef.h>

typedef struct socket socket_t;

extern int socket_init(void);
extern socket_t *socket_create(void);
extern void socket_destroy(socket_t *s);
extern void socket_abort(socket_t *s);
extern int socket_bind_ip(socket_t *s, const char *ip, int port);
extern socket_t *socket_accept(socket_t *s);
extern size_t socket_read(socket_t *s, char *buffer, size_t size);
extern size_t socket_write(socket_t *s, char *buffer, size_t size);

#endif
