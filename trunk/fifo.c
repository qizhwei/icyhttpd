#include "fifo.h"
#include "list.h"
#include "process.h"
#include <string.h>
#include <assert.h>

#define STATE_TRANSFER (0)
#define STATE_EOF (1)
#define STATE_ABORT (2)

// the pending_operation field only have meaning in transfer state
#define OPERATION_NONE (0)
#define OPERATION_READ (1)
#define OPERATION_WRITE (2)

typedef struct block {

	// async object for blocking operation, see process.h.
	async_t async;

	// list entry for pushing this block into the read or write list of a fifo.
	list_t entry;

	// points to a buffer, this buffer MUST be hold during the blocking operation.
	void *buffer;
	size_t size;

	// -1 indicates failure, notice that there MUST be no context switching
	// between the assignment of failure and the removal of failed block.
	//
	// any non-negative integer indicates how much bytes were trasnferred.
	//
	// if the result remains 0 after being removed, it indicates an EOF
	// an EOF can only happen once in a fifo, after which all read and write
	// operation SHOULD return failure.
	ssize_t result;
} block_t;

static inline void block_init(block_t *b, void *buffer, size_t size)
{
	b->buffer = buffer;
	b->size = size;
	b->result = 0;
}

void fifo_init(fifo_t *f)
{
	f->state = STATE_TRANSFER;
	f->pending_operation = OPERATION_NONE;
	list_init(&f->pending_list);
}

MAYFAIL(-1) ssize_t fifo_read(fifo_t *f, void *buffer, size_t size)
{
	block_t rb, *wb;
	size_t read_size, write_size;

	block_init(&rb, buffer, size);

	while (1) {
		if (f->state == STATE_TRANSFER) {
			if (f->pending_operation == OPERATION_WRITE) {
				wb = CONTAINER_OF(f->pending_list.next, block_t, entry);
				if (wb->size == 0) {
					list_remove(&wb->entry);
					process_unblock(&wb->async);

					while (!list_empty(&f->pending_list)) {
						wb = CONTAINER_OF(f->pending_list.next, block_t, entry);
						wb->result = -1;
						list_remove(&wb->entry);
						process_unblock(&wb->async);
					}

					f->state = STATE_EOF;
					continue;
				}

				read_size = rb.size - rb.result;
				write_size = wb->size - wb->result;

				if (read_size >= write_size) {
					memcpy(rb.buffer + rb.result, wb->buffer + wb->result, write_size);
					rb.result += write_size;
					wb->result += write_size;
					list_remove(&wb->entry);
					process_unblock(&wb->async);
					if (list_empty(&f->pending_list))
						f->pending_operation = OPERATION_NONE;
					continue;
				} else {
					memcpy(rb.buffer + rb.result, wb->buffer + wb->result, read_size);
					rb.result += read_size;
					wb->result += read_size;
				}
			}

			if (rb.size != rb.result) {
				assert(f->pending_operation == OPERATION_NONE);
				f->pending_operation = OPERATION_READ;
				list_push_back(&f->pending_list, &rb.entry);
				process_block(&rb.async);
			}
			break;
		} else if (f->state == STATE_EOF) {
			if (rb.result == 0)
				f->state == STATE_ABORT;
			break;
		} else {
			rb.result = -1;
			break;
		}
	}

	return rb.result;
}

MAYFAIL(-1) ssize_t fifo_write(fifo_t *f, void *buffer, size_t size)
{
	block_t wb, *rb;
	size_t write_size, read_size;

	block_init(&wb, buffer, size);

	while (1) {
		if (f->state == STATE_TRANSFER) {
			if (f->pending_operation == OPERATION_READ) {
				rb = CONTAINER_OF(f->pending_list.next, block_t, entry);
				if (wb.size == 0) {
					if (rb->result > 0)
						f->state = STATE_EOF;
					else
						f->state = STATE_ABORT;

					list_remove(&rb->entry);
					process_unblock(&rb->async);

					while (!list_empty(&f->pending_list)) {
						rb = CONTAINER_OF(f->pending_list.next, block_t, entry);
						if (f->state == STATE_EOF) {
							rb->result = 0;
							f->state = STATE_ABORT;
						} else {
							rb->result = -1;
						}
						list_remove(&rb->entry);
						process_unblock(&rb->async);
					}

					continue;
				}

				read_size = rb->size - rb->result;
				write_size = wb.size - wb.result;

				if (read_size <= write_size) {
					memcpy(rb->buffer + rb->result, wb.buffer + wb.result, read_size);
					rb->result += read_size;
					wb.result += read_size;
					list_remove(&rb->entry);
					process_unblock(&rb->async);
					if (list_empty(&f->pending_list))
						f->pending_operation = OPERATION_NONE;
					continue;
				} else {
					memcpy(rb->buffer + rb->result, wb.buffer + wb.result, write_size);
					rb->result += write_size;
					wb.result += write_size;
				}
			}

			if (wb.size != wb.result || wb.size == 0) {
				assert(f->pending_operation == OPERATION_NONE);
				f->pending_operation = OPERATION_WRITE;
				list_push_back(&f->pending_list, &wb.entry);
				process_block(&wb.async);
			}
			break;
		} else {
			wb.result = -1;
			break;
		}
	}

	return wb.result;
}
