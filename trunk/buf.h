#ifndef _BUF_H
#define _BUF_H

#include "runtime.h"
#include "str.h"
#include <stddef.h>

#define BUFFER_SIZE (8192)

typedef struct buf {
	io_proc_t *io_proc;
	void *object;
	char *current, *last;
	char buffer[BUFFER_SIZE];
} buf_t;

extern void buf_init(buf_t *u, io_proc_t *io_proc, void *object);
extern MAYFAIL(NULL) char *buf_gets(buf_t *u);
extern MAYFAIL(-1) ssize_t buf_read(buf_t *u, void *buffer, size_t size);
extern MAYFAIL(-1) int buf_put(buf_t *u, char *s);
extern MAYFAIL(-1) int buf_put_crlf(buf_t *u);
extern MAYFAIL(-1) int buf_puts(buf_t *u, char *s);
extern MAYFAIL(-1) int buf_putint(buf_t *u, int i);
extern MAYFAIL(-1) int buf_put_str(buf_t *u, str_t *s);
extern MAYFAIL(-1) ssize_t buf_write(buf_t *u, void *buffer, size_t size);
extern MAYFAIL(-1) int buf_write_from_proc(buf_t *u, io_proc_t *io_proc, void *object);
extern MAYFAIL(-1) int buf_flush(buf_t *u);

#endif
