#ifndef _FIFO_H
#define _FIFO_H

#include "list.h"
#include <stddef.h>

typedef struct fifo {
	int aborted;
	list_t read_list;
	list_t write_list;
} fifo_t;

extern void fifo_init(fifo_t *f);
extern size_t fifo_read(fifo_t *f, void *buffer, size_t size);
extern size_t fifo_write(fifo_t *f, void *buffer, size_t size);
extern void fifo_close(fifo_t *f);

#endif
