#include "process.h"
#include "mem.h"
#include "win32.h"
#include <stdlib.h>

#define STACK_COMMIT_SIZE (4096)
#define STACK_RESERVE_SIZE (16384)

struct process {
	proc_t *proc;
	LPVOID fiber;
	HANDLE timer;
	proc_t *abort_proc;
	union {
		void *user_param;
		void *abort_param;
	};
};

typedef struct share {
	proc_t *callback;
	void *param;
} share_t;

#define MAX_EVENTS (32)
#define MAX_CBS_PER_EVENT (7)

static process_t g_sched;
static HANDLE g_event[MAX_EVENTS];
static share_t g_share[MAX_EVENTS][MAX_CBS_PER_EVENT + 1] = {0};
static int g_idx_event = 0, g_idx_cb = 0;

static void process_switch(process_t *process)
{
	SwitchToFiber(process->fiber);
}

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

static void CALLBACK switch_proc(ULONG_PTR param)
{
	process_switch((process_t *)param);
}

int process_create(proc_t *proc, void *param)
{
	process_t *process = mem_alloc(sizeof(process_t));

	if (process == NULL)
		return -1;

	process->proc = proc;
	process->user_param = param;
	process->abort_proc = NULL;

	process->fiber = CreateFiberEx(STACK_COMMIT_SIZE, STACK_RESERVE_SIZE, 0, &fiber_proc, process);
	if (process->fiber == NULL) {
		mem_free(process);
		return -1;
	}

	process->timer = CreateWaitableTimer(NULL, TRUE, NULL);
	if (process->timer == NULL) {
		DeleteFiber(process->fiber);
		mem_free(process);
		return -1;
	}

	if (!QueueUserAPC(&switch_proc, GetCurrentThread(), (ULONG_PTR)process)) {
		// TODO: fatal error
	}

	return 0;
}

process_t *process_current(void)
{
	return GetFiberData();
}

static void CALLBACK exit_proc(ULONG_PTR param)
{
	DeleteFiber((PVOID)param);
}

void process_exit(void)
{
	process_t *process = process_current();

	// queue an APC to free the fiber, this call should not fail
	if (!QueueUserAPC(&exit_proc, GetCurrentThread(), (ULONG_PTR)process->fiber)) {
		// TODO: fatal error
	}

	// free all resources allocated other than the fiber
	CloseHandle(&process->timer);
	mem_free(process);

	// switch to the scheduling process
	process_switch(&g_sched);
}

void *process_share_event(proc_t *callback, void *param)
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

void process_block(async_t *async)
{
	process_t *process = process_current();
	async->process = process;
	process_switch(&g_sched);

	if (process->abort_proc) {
		if (!CancelWaitableTimer(process->timer)) {
			// TODO: fatal error
		}
		process->abort_proc = NULL;
	}
}

void process_unblock(async_t *async)
{
	process_switch(async->process);
}

static void CALLBACK timer_proc(void *param, DWORD low, DWORD high)
{
	process_t *process = param;
	process->abort_proc(process->abort_param);
	process->abort_proc = NULL;
}

int process_timeout(int milliseconds, proc_t *abort_proc, void *param)
{
	process_t *process = process_current();
	LARGE_INTEGER due;

	due.QuadPart = -10000LL * milliseconds;
	process->abort_proc = abort_proc;
	process->abort_param = param;

	if (!SetWaitableTimer(process->timer, &due, 0, &timer_proc, process, FALSE))
		return -1;

	return 0;
}
