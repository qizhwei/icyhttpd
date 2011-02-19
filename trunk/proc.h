#ifndef _PROC_H
#define _PROC_H

#include <sys/types.h>
#include <stddef.h>

typedef void proc_t(void *u);
typedef ssize_t io_proc_t(void *u, void *buffer, size_t size);

#endif
