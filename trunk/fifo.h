#ifndef _FIFO_H
#define _FIFO_H

#include "proc.h"
#include "list.h"
#include <stddef.h>

typedef struct fifo {
	char state;
	char pending_operation;
	list_t pending_list;
} fifo_t;

extern void fifo_init(fifo_t *f);
extern ssize_t fifo_read(fifo_t *f, void *buffer, size_t size);
extern ssize_t fifo_write(fifo_t *f, void *buffer, size_t size);

#endif
