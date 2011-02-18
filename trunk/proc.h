#ifndef _PROC_H
#define _PROC_H

#include <stddef.h>

typedef void proc_t(void *u);
typedef size_t io_proc_t(void *u, void *buffer, size_t size);

#endif
