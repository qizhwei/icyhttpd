#include "shrimp.h"
#include "semaphore.h"
#include "timer.h"
#include "win32.h"
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#define STACK_COMMIT_SIZE (4096)
#define STACK_RESERVE_SIZE (16384)

struct shrimp {
	shrimp_proc_t *proc;
	void *user_param;
	LPVOID fiber;
	timer_t timer;
};

typedef struct share {
	shrimp_proc_t *callback;
	void *param;
} share_t;

#define MAX_EVENTS (32)
#define MAX_CBS_PER_EVENT (7)

static shrimp_t g_sched;
static HANDLE g_event[MAX_EVENTS];
static share_t g_share[MAX_EVENTS][MAX_CBS_PER_EVENT + 1] = {0};
static int g_idx_event = 0, g_idx_cb = 0;

int shrimp_init(void)
{
	g_sched.fiber = ConvertThreadToFiber(&g_sched);

	if (g_sched.fiber == NULL)
		return -1;

	return 0;
}

void shrimp_loop(void)
{
	while (1) {
		int cnt = (g_idx_cb == 0 ? g_idx_event : MAX_EVENTS);

		if (cnt == 0) {
			SleepEx(INFINITE, TRUE);
		} else {
			DWORD result = WaitForMultipleObjectsEx(cnt, g_event, FALSE, INFINITE, TRUE);
			if (result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + cnt) {
				share_t *share = &g_share[result - WAIT_OBJECT_0][0];
				do
					share->callback(share->param);
				while ((++share)->callback);
			}
		}
	}
}

static void CALLBACK fiber_proc(PVOID param)
{
	shrimp_t *shrimp = param;
	shrimp->proc(shrimp->user_param);
	shrimp_exit();
}

handle_t shrimp_create(shrimp_proc_t *proc, void *param)
{
	shrimp_t *shrimp = malloc(sizeof(shrimp_t));

	if (shrimp == NULL)
		return NULL;

	shrimp->proc = proc;
	shrimp->user_param = param;
	shrimp->fiber = CreateFiberEx(STACK_COMMIT_SIZE, STACK_RESERVE_SIZE, 0, &fiber_proc, shrimp);

	if (shrimp->fiber == NULL) {
		free(shrimp);
		return NULL;
	}

	if (timer_init(&shrimp->timer)) {
		DeleteFiber(shrimp->fiber);
		free(shrimp);
		return NULL;
	}

	shrimp_ready(shrimp);
	return shrimp;
}

handle_t shrimp_current(void)
{
	return GetFiberData();
}

static void CALLBACK exit_proc(ULONG_PTR param)
{
	DeleteFiber((PVOID)param);
}

void shrimp_exit(void)
{
	shrimp_t *s = shrimp_current();

	// queue an APC to free the fiber, this call should not fail
	if (!QueueUserAPC(&exit_proc, GetCurrentThread(), (ULONG_PTR)s->fiber)) {
		// TODO: fatal error
	}

	// free all resources allocated other than the fiber
	timer_uninit(&s->timer);
	free(s);

	// switch to the scheduling shrimp
	shrimp_switch(&g_sched);

	// never be here
	assert(0);
}

void shrimp_switch(handle_t shrimp)
{
	shrimp_t *s = shrimp;
	SwitchToFiber(s->fiber);
}

static void CALLBACK switch_proc(ULONG_PTR param)
{
	shrimp_switch((handle_t)param);
}

void shrimp_ready(handle_t shrimp)
{
	if (!QueueUserAPC(&switch_proc, GetCurrentThread(), (ULONG_PTR)shrimp)) {
		// TODO: fatal error
	}
}

void shrimp_block(void)
{
	shrimp_switch(&g_sched);
}

handle_t shrimp_timer(int milliseconds)
{
	shrimp_t *s = shrimp_current();
	handle_t timer = &s->timer;

	if (timer_set(timer, milliseconds)) {
		// TODO: fatal error
	}

	return timer;
}

int shrimp_wait(int n, int m, ...)
{
	va_list vl;
	int result;

	va_start(vl, m);
	result = semaphore_wait(n, m, vl);
	va_end(vl);

	return result;
}

void * shrimp_share_event(shrimp_proc_t *callback, void *param)
{
	HANDLE event;

	if (g_idx_cb == 0) {
		event = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (event == NULL)
			return NULL;
		g_event[g_idx_event] = event;
	} else if (g_idx_cb >= MAX_CBS_PER_EVENT) {
		return NULL;
	} else {
		event = g_event[g_idx_event];
	}

	g_share[g_idx_event][g_idx_cb].callback = callback;
	g_share[g_idx_event][g_idx_cb].param = param;

	if (g_idx_event >= MAX_EVENTS - 1) {
		g_idx_event = 0;
		++g_idx_cb;
	} else {
		++g_idx_event;
	}

	return event;
}
