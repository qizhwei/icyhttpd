#include "buf.h"
#include <stddef.h>
#include <string.h>

void buf_init(buf_t *u, io_proc_t *io_proc, void *object)
{
	u->io_proc = io_proc;
	u->object = object;
	u->current = NULL;
	u->last = NULL;
}

char *buf_gets(buf_t *u)
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
		if (cur_size == BUFFER_SIZE)
			return NULL;

		read_size = u->io_proc(u->object, last, BUFFER_SIZE - cur_size);
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

size_t buf_read(buf_t *u, void *buffer, size_t size)
{
	size_t cur_size = u->last - u->current;

	if (cur_size) {
		if (cur_size < size)
			size = cur_size;
		memcpy(buffer, u->current, size);
		u->current += size;
		return size;
	} else {
		return u->io_proc(u->object, buffer, size);
	}
}

int buf_put(buf_t *u, char *s)
{
	size_t s_size = strlen(s);
	return (s_size && !buf_write(u, s, s_size)) ? -1 : 0;
}

int buf_puts(buf_t *u, char *s)
{
	return (buf_put(u, s) || !buf_write(u, "\r\n", 2)) ? -1 : 0;
}

size_t buf_write(buf_t *u, void *buffer, size_t size)
{
	size_t cur_size = u->last - u->current;
	size_t rem_size;
	size_t result;
	size_t result0;

	if (cur_size) {
		rem_size = BUFFER_SIZE - cur_size;
		if (size < rem_size) {
			memcpy(u->last, buffer, size);
			u->last += size;
			return size;
		} else {
			memcpy(u->last, buffer, rem_size);
			result = u->io_proc(u->object, u->current, BUFFER_SIZE);
			if (result == 0)
				return 0;
			buffer = (char *)buffer + rem_size;
			size -= rem_size;
		}
	} else {
		result = 0;
	}

	if (size >= BUFFER_SIZE) {
		result0 = u->io_proc(u->object, buffer, size);
		if (result0 == 0)
			return 0;
		result += result0;
	} else {
		memcpy(u->buffer, buffer, size);
		u->current = u->buffer;
		u->last = u->buffer + size;
		result += size;
	}

	return result;
}

int buf_flush(buf_t *u)
{
	size_t cur_size = u->last - u->current;

	if (cur_size) {
		if (!u->io_proc(u->object, u->current, cur_size))
			return -1;
		u->current += cur_size;
	}

	return 0;
}