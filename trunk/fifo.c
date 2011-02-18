#include "fifo.h"
#include "list.h"
#include "process.h"
#include <string.h>

typedef struct block {
	async_t async;
	list_t entry;
	void *buffer;
	size_t size;
	size_t transferred;
} block_t;

static inline size_t min_size(size_t a, size_t b)
{
	return a < b ? a : b;
}

static inline void block_init(block_t *b, void *buffer, size_t size)
{
	b->buffer = buffer;
	b->size = size;
	b->transferred = 0;
}

void fifo_init(fifo_t *f)
{
	f->aborted = 0;
	list_init(&f->read_list);
	list_init(&f->write_list);
}

size_t fifo_read(fifo_t *f, void *buffer, size_t size)
{
	block_t b, *p;
	size_t copy_size;

	block_init(&b, buffer, size);

	while (!list_empty(&f->write_list)) {
		p = CONTAINER_OF(f->write_list.next, block_t, entry);
		copy_size = min_size(b.size - b.transferred, p->size - p->transferred);

		memcpy(b.buffer + b.transferred, p->buffer + p->transferred, copy_size);
		b.transferred += copy_size;
		p->transferred += copy_size;

		if (p->transferred == p->size) {
			list_remove(&p->entry);
			process_unblock(&p->async);
		}

		if (b.transferred == b.size)
			return b.transferred;
	}

	if (b.transferred == 0 && !f->aborted) {
		list_push_back(&f->read_list, &b.entry);
		process_block(&b.async);
	}

	return b.transferred;
}

size_t fifo_write(fifo_t *f, void *buffer, size_t size)
{
	block_t b, *p;
	size_t copy_size;

	block_init(&b, buffer, size);

	while (!list_empty(&f->read_list)) {
		p = CONTAINER_OF(f->read_list.next, block_t, entry);
		copy_size = min_size(b.size - b.transferred, p->size - p->transferred);
		memcpy(p->buffer + p->transferred, b.buffer + b.transferred, copy_size);
		b.transferred += copy_size;
		p->transferred += copy_size;

		if (p->transferred == p->size || b.transferred == b.size) {
			list_remove(&p->entry);
			process_unblock(&p->async);
		}

		if (b.transferred == b.size)
			return b.transferred;
	}

	if (size == 0) {
		f->aborted = 1;
	} else if (b.transferred != b.size && !f->aborted) {
		list_push_back(&f->write_list, &b.entry);
		process_block(&b.async);
	}

	return b.transferred;
}

void fifo_close(fifo_t *f)
{
	fifo_write(f, NULL, 0);
}
