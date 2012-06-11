#ifndef _FIFO_H
#define _FIFO_H

#include <stddef.h>
#include "types.h"
#include "list.h"

// this file declares a set of data structures and routines for fifo component

typedef struct fifo {
	int eof;
	list_t read_list;
	list_t write_list;
} fifo_t;

extern void fifo_init(fifo_t *f);
extern int fifo_is_empty(fifo_t *f);
extern int fifo_read(fifo_t *f, char *buf, size_t size, completion_t *cb, void *u);
extern int fifo_write(fifo_t *f, const char *buf, size_t size, completion_t *cb, void *u);
extern void fifo_abort(fifo_t *f);

#endif
