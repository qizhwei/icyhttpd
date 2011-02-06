#include "semaphore.h"
#include "process.h"
#include <assert.h>
#include <stdarg.h>

typedef struct async {
	process_t *process;
	int wake_counter;
	int wake_reason;
} async_t;

typedef struct wait_block {
	async_t *async;
	list_t entry;
	int wake_reason;
} wait_block_t;

void semaphore_init(handle_t semaphore)
{
	semaphore_t *s = semaphore;
	s->available = 0;
	list_init(&s->queue);
}

static void semaphore_test(semaphore_t *s)
{
	if (s->available && !list_empty(&s->queue)) {
		list_t *entry = list_pop_front(&s->queue);
		wait_block_t *wb = CONTAINER_OF(entry, wait_block_t, entry);
		async_t *async = wb->async;
		--(s->available);
		if (--(async->wake_counter) == 0) {
			async->wake_reason = wb->wake_reason;
			process_ready(async->process);
		}
	}
}

void semaphore_signal(handle_t semaphore)
{
	semaphore_t *s = semaphore;
	++(s->available);
	semaphore_test(s);
}

int semaphore_wait(int n, int m, va_list vl)
{
	async_t async;
	wait_block_t wb[MAX_WAIT_SEMAPHORES];
	int i;

	assert(0 < m && m <= n && n <= MAX_WAIT_SEMAPHORES);

	async.process = process_current();
	async.wake_counter = m;

	for (i = 0; i < n; ++i) {
		semaphore_t *sema = va_arg(vl, semaphore_t *);
		wb[i].async = &async;
		list_push_back(&sema->queue, &wb[i].entry);
		wb[i].wake_reason = i;
		semaphore_test(sema);
	}

	process_block();
	return async.wake_reason;
}
