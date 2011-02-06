#include "process.h"
#include "semaphore.h"
#include "timer.h"
#include "win32.h"
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#define STACK_COMMIT_SIZE (4096)
#define STACK_RESERVE_SIZE (16384)

struct process {
	process_proc_t *proc;
	void *user_param;
	LPVOID fiber;
	timer_t timer;
};

typedef struct share {
	process_proc_t *callback;
	void *param;
} share_t;

#define MAX_EVENTS (32)
#define MAX_CBS_PER_EVENT (7)

static process_t g_sched;
static HANDLE g_event[MAX_EVENTS];
static share_t g_share[MAX_EVENTS][MAX_CBS_PER_EVENT + 1] = {0};
static int g_idx_event = 0, g_idx_cb = 0;

int process_init(void)
{
	g_sched.fiber = ConvertThreadToFiber(&g_sched);

	if (g_sched.fiber == NULL)
		return -1;

	return 0;
}

void process_loop(void)
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
	process_t *process = param;
	process->proc(process->user_param);
	process_exit();
}

handle_t process_create(process_proc_t *proc, void *param)
{
	process_t *process = malloc(sizeof(process_t));

	if (process == NULL)
		return NULL;

	process->proc = proc;
	process->user_param = param;
	process->fiber = CreateFiberEx(STACK_COMMIT_SIZE, STACK_RESERVE_SIZE, 0, &fiber_proc, process);

	if (process->fiber == NULL) {
		free(process);
		return NULL;
	}

	if (timer_init(&process->timer)) {
		DeleteFiber(process->fiber);
		free(process);
		return NULL;
	}

	process_ready(process);
	return process;
}

handle_t process_current(void)
{
	return GetFiberData();
}

static void CALLBACK exit_proc(ULONG_PTR param)
{
	DeleteFiber((PVOID)param);
}

void process_exit(void)
{
	process_t *s = process_current();

	// queue an APC to free the fiber, this call should not fail
	if (!QueueUserAPC(&exit_proc, GetCurrentThread(), (ULONG_PTR)s->fiber)) {
		// TODO: fatal error
	}

	// free all resources allocated other than the fiber
	timer_uninit(&s->timer);
	free(s);

	// switch to the scheduling process
	process_switch(&g_sched);

	// never be here
	assert(0);
}

void process_switch(handle_t process)
{
	process_t *s = process;
	SwitchToFiber(s->fiber);
}

static void CALLBACK switch_proc(ULONG_PTR param)
{
	process_switch((handle_t)param);
}

void process_ready(handle_t process)
{
	if (!QueueUserAPC(&switch_proc, GetCurrentThread(), (ULONG_PTR)process)) {
		// TODO: fatal error
	}
}

void process_block(void)
{
	process_switch(&g_sched);
}

handle_t process_timer(int milliseconds)
{
	process_t *s = process_current();
	handle_t timer = &s->timer;

	if (timer_set(timer, milliseconds)) {
		// TODO: fatal error
	}

	return timer;
}

int process_wait(int n, int m, ...)
{
	va_list vl;
	int result;

	va_start(vl, m);
	result = semaphore_wait(n, m, vl);
	va_end(vl);

	return result;
}

void * process_share_event(process_proc_t *callback, void *param)
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
