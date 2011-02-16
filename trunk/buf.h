#ifndef _BUF_H
#define _BUF_H

#include <stddef.h>

#define BUF_MAX_LINE (8192)

typedef size_t io_proc_t(void *u, void *buffer, size_t size);

typedef struct readbuf {
	io_proc_t *read_proc;
	void *object;
	char *current, *last;
	char buffer[BUF_MAX_LINE];
} readbuf_t;

typedef struct writebuf {
	// TODO
} writebuf_t;

// TODO: do we need close_proc?
// TODO: do we need blockreadbuf?

extern void readbuf_init(readbuf_t *u, io_proc_t *read_proc, void *object);
extern char *readbuf_gets(readbuf_t *u);
extern size_t readbuf_read(readbuf_t *u, void *buffer, size_t size);

#endif
