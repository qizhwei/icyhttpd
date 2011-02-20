#include "buf.h"
#include "str.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

void buf_init(buf_t *u, io_proc_t *io_proc, void *object)
{
	u->io_proc = io_proc;
	u->object = object;
	u->current = NULL;
	u->last = NULL;
}

MAYFAIL(NULL) char *buf_gets(buf_t *u)
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
		if (read_size <= 0)
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

MAYFAIL(-1) ssize_t buf_read(buf_t *u, void *buffer, size_t size)
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

MAYFAIL(-1) int buf_put(buf_t *u, char *s)
{
	size_t s_size = strlen(s);
	return (s_size && buf_write(u, s, s_size) == -1) ? -1 : 0;
}

MAYFAIL(-1) int buf_put_crlf(buf_t *u)
{
	return buf_write(u, "\r\n", 2) == -1 ? -1 : 0;
}

MAYFAIL(-1) int buf_puts(buf_t *u, char *s)
{
	return (buf_put(u, s) || buf_put_crlf(u)) ? -1 : 0;
}

MAYFAIL(-1) int buf_putint(buf_t *u, int i)
{
	char buffer[16];
	// TODO: write one `itoa' by hand to improve efficiency
	snprintf(buffer, sizeof(buffer), "%d", i);
	return buf_put(u, buffer);
}

MAYFAIL(-1) int buf_put_str(buf_t *u, str_t *s)
{
	return buf_write(u, s->buffer, s->length) == -1 ? -1 : 0;
}

MAYFAIL(-1) ssize_t buf_write(buf_t *u, void *buffer, size_t size)
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
			if (result == -1)
				return -1;
			buffer = (char *)buffer + rem_size;
			size -= rem_size;
		}
	} else {
		result = 0;
	}

	if (size >= BUFFER_SIZE) {
		result0 = u->io_proc(u->object, buffer, size);
		if (result0 == -1)
			return -1;
		result += result0;
	} else {
		memcpy(u->buffer, buffer, size);
		u->current = u->buffer;
		u->last = u->buffer + size;
		result += size;
	}

	return result;
}

MAYFAIL(-1) int buf_write_from_proc(buf_t *u, io_proc_t *io_proc, void *object)
{
	// TODO: optimize this function using the internal buffer
	char buffer[4096];
	ssize_t result;

	while (1) {
		if ((result = io_proc(object, buffer, sizeof(buffer))) == -1)
			return -1; // TODO: should we end dest-buf while a reading error occur?
		if ((result = buf_write(u, buffer, result)) <= 0)
			return result;
	}
}

MAYFAIL(-1) int buf_flush(buf_t *u)
{
	size_t cur_size = u->last - u->current;

	if (cur_size) {
		if (u->io_proc(u->object, u->current, cur_size) == -1)
			return -1;
		u->current += cur_size;
	}

	return 0;
}
