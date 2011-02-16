#include "buf.h"
#include <stddef.h>
#include <string.h>

void readbuf_init(readbuf_t *u, io_proc_t *read_proc, void *object)
{
	u->read_proc = read_proc;
	u->object = object;
	u->current = NULL;
	u->last = NULL;
}

char *readbuf_gets(readbuf_t *u)
{
	char *buffer = u->buffer;
	char *current = u->current;
	char *last = u->last;
	size_t cur_size = last - current;
	char *next = memchr(current, '\n', cur_size);
	size_t read_size;

	if (next != NULL) {
		if (next[-1] == '\r')
			next[-1] = '\0';
		next[0] = '\0';
		++next;
		u->current = next;
		return current;
	}

	memmove(buffer, current, cur_size);
	current = buffer;
	last = current + cur_size;

	while (1) {
		if (cur_size == BUF_MAX_LINE)
			return NULL;

		read_size = u->read_proc(u->object, last, BUF_MAX_LINE - cur_size);
		if (read_size == 0)
			return NULL;

		cur_size += read_size;
		next = memchr(last, '\n', cur_size);
		last += read_size;

		if (next != NULL) {
			if (next[-1] == '\r')
				next[-1] = '\0';
			next[0] = '\0';
			++next;
			u->current = next;
			u->last = last;
			return current;
		}
	}
}

size_t readbuf_read(readbuf_t *u, void *buffer, size_t size)
{
	size_t cur_size = u->last - u->current;

	if (cur_size) {
		if (cur_size < size)
			size = cur_size;
		memcpy(buffer, u->current, size);
		u->current += size;
		return size;
	} else {
		return u->read_proc(u->object, buffer, size);
	}
}
