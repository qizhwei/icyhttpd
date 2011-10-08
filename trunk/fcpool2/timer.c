#include <windows.h>
#include "types.h"
#include "timer.h"
#include "obj.h"

struct timer {
	int active;
	int cancel; // number of pending cancels
	int interval;
	callback_t *cb;
	void *u;
	HANDLE w32_timer;
};

static obj_type_t timer_type;

static void timer_uninit(timer_t *t)
{
	CloseHandle(t->w32_timer);
}

int timer_startup(void)
{
	timer_type.size = sizeof(timer_t);
	timer_type.uninit = (uninit_method_t *)&timer_uninit;

	return 0;
}

timer_t *timer_create(int interval, callback_t *cb, void *u)
{
	timer_t *t = (timer_t *)obj_alloc(&timer_type);
	if (t == NULL)
		return NULL;

	// timer object initialization begins
	t->w32_timer = CreateWaitableTimer(NULL, TRUE, NULL);
	if (t->w32_timer == NULL) {
		obj_free(t);
		return NULL;
	}

	t->active = 0;
	t->cancel = 0;
	t->interval = interval;
	t->cb = cb;
	t->u = u;
	// timer object initialization ends

	return t;
}

static void CALLBACK timer_apc(LPVOID state, DWORD low, DWORD high)
{
	timer_t *t = (timer_t *)state;
	
	if (t->cancel) {
		--t->cancel;
	} else {
		// prevent calling timer_stop() in timer callback
		t->active = 0;
		(*t->cb)(t->u);
	}

	obj_release(t);
}

int timer_start(timer_t *t)
{
	LARGE_INTEGER due_time;

	if (t->active)
		return -1;

	due_time.QuadPart = -10000LL * t->interval;
	if (!SetWaitableTimer(t->w32_timer, &due_time, 0, &timer_apc, t, FALSE))
		return -1;

	// adds reference to the timer for callback
	obj_add_ref(t);
	t->active = 1;
	return 0;
}

int timer_stop(timer_t *t)
{
	if (!t->active)
		return -1;
	
	t->active = 0;
	if (CancelWaitableTimer(t->w32_timer)) {
		// the underlying cancellation is succeeded
		// release object once since the callback will not be called
		obj_release(t);
	} else {
		// increment pending cancels
		++t->cancel;
	}

	return 0;
}
