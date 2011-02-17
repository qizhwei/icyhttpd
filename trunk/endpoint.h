#ifndef _ENDPOINT_H
#define _ENDPOINT_H

#include "node.h"

typedef struct endpoint endpoint_t;

extern endpoint_t *endpoint_create(const char *ip, int port, node_t *default_node);
extern node_t *endpoint_get_node(endpoint_t *endpoint, str_t *host);

#endif
