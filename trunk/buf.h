#ifndef _BUF_H
#define _BUF_H

#include "proc.h"
#include <stddef.h>

#define BUFFER_SIZE (8192)

typedef struct buf {
	io_proc_t *io_proc;
	void *object;
	char *current, *last;
	char buffer[BUFFER_SIZE];
} buf_t;

extern void buf_init(buf_t *u, io_proc_t *io_proc, void *object);
extern char *buf_gets(buf_t *u);
extern size_t buf_read(buf_t *u, void *buffer, size_t size);
extern int buf_put(buf_t *u, char *s);
extern int buf_puts(buf_t *u, char *s);
extern size_t buf_write(buf_t *u, void *buffer, size_t size);
extern int buf_flush(buf_t *u);

#endif
