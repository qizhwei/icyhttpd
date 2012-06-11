#ifndef _INSTANCE_H
#define _INSTANCE_H

#include <stddef.h>
#include "types.h"

typedef struct inst inst_t;

extern void inst_startup(void);
extern inst_t *inst_create(struct pool *pool);
extern void inst_abort(inst_t *i);
extern int inst_write(inst_t *i, const char *buf, size_t size, completion_t *cb, void *u);

#endif
