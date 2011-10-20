#include <stdlib.h>
#include <windows.h>
#include <assert.h>
#include "sched.h"

#define SMT_THREAD_MAX (4)
#define STACK_COMMIT_SIZE (4096)
#define STACK_RESERVE_SIZE (16384)

typedef struct sched_worker worker_t;

typedef struct join_struct {
	sched_environ_t env;
	LPVOID w32_fiber;
	worker_t *w;
	void *result;
} join_struct_t;

struct sched_worker {
	volatile LONG ref;
	volatile LONG detach;
	sched_worker_cb_t *cb;
	void *u;
	LPVOID w32_fiber;
	void *result;
	join_struct_t *js;
};

typedef void post_worker_cb_t(void *param);

typedef struct environ {
	sched_environ_t env;
	HANDLE w32_thread;
	LPVOID w32_fiber;
	post_worker_cb_t *post_cb;
	void *post_u;
} environ_t;

#define current_env ((environ_t *)TlsGetValue(w32_tlsidx))

static DWORD w32_tlsidx;
static HANDLE w32_iocp;
static environ_t st_env;
static environ_t smt_env[SMT_THREAD_MAX];

static void CALLBACK fiber_apc(ULONG_PTR param)
{
	SwitchToFiber((LPVOID)param);
}

static void exit_worker(void *param)
{
	worker_t *w = (worker_t *)param;

	// delete fiber
	DeleteFiber(w->w32_fiber);

	// unwait pending join
	if (InterlockedCompareExchange(&w->detach, 1, 0) != 0) {
		w->js->result = w->result;
		switch (w->js->env) {
		case SCHED_ENVIRON_ST:
			if (!QueueUserAPC(&fiber_apc, st_env.w32_thread, (ULONG_PTR)w->js->w32_fiber))
				abort();
			break;
		case SCHED_ENVIRON_SMT:
			assert(!"not implemented");
			break;
		default:
			assert(!"never be here");
		}
	}

	sched_detach(w);
}

// worker entry
static void CALLBACK worker_entry(LPVOID param)
{
	worker_t *w = (worker_t *)param;
	environ_t *env;
	w->result = (*w->cb)(w->u);
	// notice that env may change
	env = current_env;
	env->post_cb = &exit_worker;
	env->post_u = w;
	SwitchToFiber(current_env->w32_fiber);
}

static void flush_worker(void)
{
	environ_t *env = current_env;
	post_worker_cb_t *cb;
	while (env->post_cb != NULL) {
		cb = env->post_cb;
		env->post_cb = NULL;
		(*cb)(env->post_u);

	}
}

// ST thread entry
static void CALLBACK w32_st_entry(LPVOID param)
{
	// message loop
	while (1) {
		flush_worker();
		SleepEx(INFINITE, TRUE);
	}
}

// SMT thread entry
static DWORD CALLBACK w32_smt_entry(LPVOID param)
{
	environ_t *env = (environ_t *)param;
	DWORD size;
	ULONG_PTR key;
	LPOVERLAPPED overlapped;
	BOOL success;

	// initialize environment
	if (!TlsSetValue(w32_tlsidx, env))
		abort();
	env->w32_fiber = ConvertThreadToFiber(NULL);
	if (env->w32_fiber == NULL)
		abort();

	// message loop
	while (1) {
		flush_worker();
		success = GetQueuedCompletionStatus(w32_iocp, &size, &key, &overlapped, INFINITE);
		if (success == FALSE && overlapped == NULL)
			abort();
	}

	return 0;
}

int sched_startup(void)
{
	int i;
	DWORD dummy;

	// allocate TLS
	w32_tlsidx = TlsAlloc();
	if (w32_tlsidx == TLS_OUT_OF_INDEXES)
		return -1;

	// initialize ST environment
	if (!TlsSetValue(w32_tlsidx, &st_env))
		goto err;
	if (ConvertThreadToFiber(NULL) == NULL)
		goto err;
	st_env.w32_fiber = CreateFiberEx(STACK_COMMIT_SIZE, STACK_RESERVE_SIZE, 0, &w32_st_entry, NULL);
	if (st_env.w32_fiber == NULL)
		goto err;
	st_env.env = SCHED_ENVIRON_ST;
	st_env.post_cb = NULL;
	if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(),
		&st_env.w32_thread, 0, FALSE, DUPLICATE_SAME_ACCESS))
		goto err;

	// initialize SMT environment
	w32_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (w32_iocp == NULL)
		goto err0;
	for (i = 0; i < SMT_THREAD_MAX; ++i) {
		smt_env[i].env = SCHED_ENVIRON_SMT;
		smt_env[i].post_cb = NULL;
		smt_env[i].w32_thread = CreateThread(NULL, 0, &w32_smt_entry, &smt_env[i], 0, &dummy);
		if (smt_env[i].w32_thread == NULL)
			goto rollback;
	}

	return 0;
rollback:
	for (--i; i >= 0; --i) {
		TerminateThread(smt_env[i].w32_thread, 1);
		CloseHandle(smt_env[i].w32_thread);
	}
err0:
	CloseHandle(smt_env[i].w32_thread);
err:
	TlsFree(w32_tlsidx);
	return -1;
}

worker_t *sched_begin(sched_environ_t env, sched_worker_cb_t *cb, void *u)
{
	worker_t *w = (worker_t *)malloc(sizeof(worker_t));
	if (w == NULL)
		return NULL;

	w->ref = 2; // one for user, one for worker
	w->detach = 0;
	w->cb = cb;
	w->u = u;
	w->w32_fiber = CreateFiberEx(STACK_COMMIT_SIZE, STACK_RESERVE_SIZE, 0, &worker_entry, w);
	if (w->w32_fiber == NULL)
		goto err;

	switch (env) {
	case SCHED_ENVIRON_ST:
		if (!QueueUserAPC(&fiber_apc, st_env.w32_thread, (ULONG_PTR)w->w32_fiber))
			goto err0;
		break;
	case SCHED_ENVIRON_SMT:
		assert(!"not implemented");
		break;
	default:
		goto err0;
	}

	return w;
err0:
	DeleteFiber(w->w32_fiber);
err:
	free(w);
	return NULL;
}

void sched_detach(worker_t *w)
{
	if (InterlockedDecrement(&w->ref) == 0)
		free(w);
}

static void join_worker(void *param)
{
	worker_t *w = (worker_t *)param;
	join_struct_t *js;

	if (InterlockedCompareExchange(&w->detach, 1, 0) == 0) {
		// worker not finished, just wait
		sched_detach(w);
	} else {
		// worker finished, join now
		js = w->js;
		js->result = w->result;
		sched_detach(w);
		SwitchToFiber(js->w32_fiber);
	}
}

void *sched_join(worker_t *w)
{
	environ_t *env = current_env;
	join_struct_t js;
	js.env = env->env;
	js.w32_fiber = GetCurrentFiber();
	js.w = w;
	w->js = &js;
	env->post_cb = &join_worker;
	env->post_u = w;
	SwitchToFiber(env->w32_fiber);
	return js.result;
}

void sched_loop_forever(void)
{
	SwitchToFiber(st_env.w32_fiber);
}
