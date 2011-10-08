#ifndef _PRIVATE_H
#define _PRIVATE_H

// this file contains internal data structures for object types
// shared across multiple modules

#include <windows.h>
#include "list.h"
#include "pool.h"
#include "inst.h"
#include "req.h"
#include "timer.h"
#include "fifo.h"

// pool object

struct pool {
	// configurations
	char *cmd_line;
	int queue_length;
	int max_instances;
	int idle_time;
	int max_requests;

	// pending request queue
	list_t req_list;
	int req_cnt;

	// pending instance stack
	list_t inst_list;

	// total running instances
	int run_cnt;

	// win32 handles
	HANDLE w32_job;
};

// instance object

#define INST_IDLE (0)
#define INST_RUNNING (1)
#define INST_POOLING (2)
#define INST_RUNDOWN (3)

struct inst {
	// instance state
	int state;

	// referenced pointer to pool object
	// always valid (even in rundown state for marriage)
	pool_t *pool;

	union {
		// list entry for instance stack in pool object
		// valid when state == INST_POOLING
		list_t pool_entry;

		// referenced pointer to request object
		// valid when state == INST_RUNNING
		req_t *req;
	};

	// remaining request count this instance can handle
	int remain;

	// timer for pooling expiration
	timer_t *timer;

	// win32 handles
	HANDLE w32_pipe;
	HANDLE w32_process;
};

// request object

#define REQ_IDLE (0)
#define REQ_PENDING (1)
#define REQ_BEGIN (2)
#define REQ_ACTIVE (3)
#define REQ_END (4)

struct req {
	int state;

	// fifos
	fifo_t f_params;
	fifo_t f_stdin;
	fifo_t f_stdout;

	union {
		// referenced pointer to pool object
		// valid on REQ_PENDING
		pool_t *pool;

		// referenced pointer to inst handle object
		// valid on REQ_BEGIN or REQ_ACTIVE
		inst_handle_t *hi;
	};

	// pending request queue entry
	// valid on REQ_PENDING
	list_t rq_entry;

	// request begin callback
	// valid on REQ_PENDING or REQ_BEGIN
	req_begin_cb_t *begin_cb;
	void *begin_u;
};

#endif
