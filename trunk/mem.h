#ifndef _MEM_H
#define _MEM_H

#include <stddef.h>

extern int mem_init(void);
extern void *mem_alloc(size_t size);
extern void mem_free(void *p);

#endif
