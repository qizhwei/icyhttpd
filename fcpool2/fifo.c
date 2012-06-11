#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <windows.h>
#include "types.h"
#include "fifo.h"

typedef struct ffblock {
	list_t entry;
	char *buf;
	size_t size;
	ssize_t trans;
	completion_t *cb;
	void *u;
} ffblock_t;

void fifo_init(fifo_t *f)
{
	f->eof = 0;
	list_init(&f->read_list);
	list_init(&f->write_list);
}

int fifo_is_empty(fifo_t *f)
{
	if (!list_is_empty(&f->read_list))
		return 0;
	if (!list_is_empty(&f->write_list))
		return 0;
	return 1;
}


static void CALLBACK complete_apc(ULONG_PTR param)
{
	ffblock_t *b = (ffblock_t *)param;
	(*b->cb)(b->u, b->trans);
	free(b);
}

static void complete(ffblock_t *b)
{
	list_remove(&b->entry);
	if (!QueueUserAPC(&complete_apc, GetCurrentThread(), (ULONG_PTR)b)) {
		assert(!"failed to complete fifo");
	}
}

static void transfer(fifo_t *f)
{
	ffblock_t *rb, *wb;
	size_t rlen, wlen;

	while (!list_is_empty(&f->read_list) && !list_is_empty(&f->write_list)) {
		rb = container_of(f->read_list.next, ffblock_t, entry);
		wb = container_of(f->write_list.next, ffblock_t, entry);

		if (wb->size == 0) {
			complete(rb);
			complete(wb);
			fifo_abort(f);
		} else {
			rlen = rb->size - rb->trans;
			wlen = wb->size - wb->trans;

			if (rlen > wlen)
				rlen = wlen;

			memcpy(&rb->buf[rb->trans], &wb->buf[wb->trans], rlen);
			rb->trans += rlen;
			wb->trans += rlen;

			if (rb->size == rb->trans)
				complete(rb);
			if (wb->size == wb->trans)
				complete(wb);
		}
	}
}

int fifo_read(fifo_t *f, char *buf, size_t size, completion_t *cb, void *u)
{
	ffblock_t *b;

	if (size == 0 || f->eof)
		return -1;
	if ((b = (ffblock_t *)malloc(sizeof(*b))) == NULL)
		return -1;

	list_add_tail(&f->read_list, &b->entry);
	b->buf = buf;
	b->size = size;
	b->trans = 0;
	b->cb = cb;
	b->u = u;

	transfer(f);
	return 0;
}

int fifo_write(fifo_t *f, const char *buf, size_t size, completion_t *cb, void *u)
{
	ffblock_t *b;

	if (f->eof)
		return -1;
	if ((b = (ffblock_t *)malloc(sizeof(*b))) == NULL)
		return -1;

	list_add_tail(&f->write_list, &b->entry);
	b->buf = (char *)buf;
	b->size = size;
	b->trans = 0;
	b->cb = cb;
	b->u = u;

	transfer(f);
	return 0;
}

void fifo_abort(fifo_t *f)
{
	ffblock_t *b;

	while (!list_is_empty(&f->read_list)) {
		b = container_of(f->read_list.next, ffblock_t, entry);
		b->trans = -1;
		complete(b);
	}

	while (!list_is_empty(&f->write_list)) {
		b = container_of(f->write_list.next, ffblock_t, entry);
		b->trans = -1;
		complete(b);
	}

	f->eof = 1;
}
