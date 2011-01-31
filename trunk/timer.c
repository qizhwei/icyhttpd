#include "timer.h"
#include "semaphore.h"
#include "win32.h"

int timer_init(handle_t timer)
{
	timer_t *t = timer;
	semaphore_init(&t->semaphore);
	t->os_timer = CreateWaitableTimer(NULL, TRUE, NULL);

	if (t->os_timer == NULL)
		return -1;

	return 0;
}

void timer_uninit(handle_t timer)
{
	timer_t *t = timer;
	CloseHandle(t->os_timer);
}

static void CALLBACK timer_proc(void *state, DWORD low, DWORD high)
{
	timer_t *t = state;
	semaphore_signal(&t->semaphore);
}

int timer_set(handle_t timer, int milliseconds)
{
	timer_t *t = timer;
	LARGE_INTEGER due;
	due.QuadPart = -10000LL * milliseconds;

	if (!SetWaitableTimer(t->os_timer, &due, 0, &timer_proc, t, FALSE))
		return -1;

	return 0;
}
