#ifndef _PROCESS_H
#define _PROCESS_H

#include "runtime.h"

extern MAYFAIL(-1) int process_init(void);
extern void process_loop(void);

typedef struct process process_t;

typedef struct async {
	process_t *process;
} async_t;

extern MAYFAIL(-1) int process_create(proc_t *proc, void *param);
extern NOFAIL process_t *process_current(void);
extern void process_exit(void);
extern MAYFAIL(NULL) void *process_share_event(proc_t *callback, void *param);
extern void process_block(async_t *async);
extern void process_unblock(async_t *async);
extern void process_unblock_now(async_t *async);
extern void process_timeout(int milliseconds, proc_t *abort_proc, void *param);

#endif
