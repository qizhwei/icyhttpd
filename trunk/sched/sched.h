#ifndef _SCHED_H
#define _SCHED_H

// defines environment for workers
typedef enum sched_environ {
	SCHED_ENVIRON_ST,	// single thread
	SCHED_ENVIRON_SMT,	// symmetric multi-thread
} sched_environ_t;

// worker function prototype
typedef void *sched_worker_cb_t(void *u);

// worker handle
typedef struct sched_worker sched_worker_t;

// functions
extern int sched_startup(void);
extern sched_worker_t *sched_begin(sched_environ_t env, sched_worker_cb_t *cb, void *u);
extern void sched_detach(sched_worker_t *w);
extern void *sched_join(sched_worker_t *w);
extern void sched_loop_forever(void);

#endif
