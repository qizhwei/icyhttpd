#include "event.h"
#include "list.h"
#include "process.h"

#define STATE_MANUAL (0)
#define STATE_MANUAL_SIGNALED (1)
#define STATE_AUTO (2)
#define STATE_AUTO_SIGNALED (3)

typedef struct wait_block {
	async_t async;
	list_t entry;
} wait_block_t;

void event_init(event_t *e, char manual_reset, char signaled)
{
	e->manual_reset = manual_reset;
	e->signaled = signaled;
	list_init(&e->wait_list);
}

void event_set(event_t *e)
{
	list_t *entry = e->wait_list.next, *next;
	wait_block_t *wb;

	if (entry == &e->wait_list) {
		e->signaled = 1;
	} else if (e->manual_reset) {
		do {
			wb = CONTAINER_OF(entry, wait_block_t, entry);
			process_unblock(&wb->async);
			next = entry->next;
			list_remove(entry);
		} while ((entry = next) != &e->wait_list);
		e->signaled = 1;
	} else {
		wb = CONTAINER_OF(entry, wait_block_t, entry);
		process_unblock(&wb->async);
		list_remove(entry);
	}
}

void event_reset(event_t *e)
{
	e->signaled = 0;
}

void event_wait(event_t *e)
{
	wait_block_t wb;

	if (e->signaled) {
		if (!e->manual_reset)
			e->signaled = 0;
	} else {
		list_push_back(&e->wait_list, &wb.entry);
		process_block(&wb.async);
	}
}
