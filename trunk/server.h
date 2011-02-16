#ifndef _SERVER_H
#define _SERVER_H

typedef struct endpoint endpoint_t;

extern endpoint_t *endpoint_create(const char *ip, int port);

#endif
