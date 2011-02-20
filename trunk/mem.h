#ifndef _MEM_H
#define _MEM_H

#include <stddef.h>
#include "runtime.h"

extern void mem_init(void);
extern NOFAIL void *mem_alloc(size_t size);
extern void mem_free(void *p);

#endif
