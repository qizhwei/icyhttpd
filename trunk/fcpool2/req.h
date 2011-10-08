#ifndef _REQ_H
#define _REQ_H

#include <stddef.h>
#include "types.h"

typedef struct req req_t;

typedef void req_begin_cb_t(void *u, int err);

extern int req_startup(void);
extern req_t *req_create(void);
extern void req_abort(req_t *r);
extern int req_begin(req_t *r, pool_t *pool, req_begin_cb_t *cb, void *u);
extern int req_read(req_t *r, char *buf, size_t size, completion_t *cb, void *u);
extern int req_write(req_t *r, const char *buf, size_t size, completion_t *cb, void *u);
extern int req_write_param(req_t *r, const char *buf, size_t size, completion_t *cb, void *u);

#endif
