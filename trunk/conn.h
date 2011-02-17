#ifndef _CONN_H
#define _CONN_H

#include "endpoint.h"
#include "socket.h"

extern int conn_create(endpoint_t *e, socket_t *s);

#endif
