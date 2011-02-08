#ifndef _PROCESS_H
#define _PROCESS_H

#include "handle.h"

extern int process_init(void);
extern void process_loop(void);

typedef struct process process_t;
typedef void process_proc_t(void *param);

typedef struct async {
	// the process to switch to, or NULL if cancelled
	process_t *process;
} async_t;

extern int process_create(process_proc_t *proc, void *param);
extern process_t *process_current(void);
extern void process_exit(void);
extern void *process_share_event(process_proc_t *callback, void *param);
extern void process_block(async_t *async);
extern void process_switch(process_t *process);
extern int process_timeout(int milliseconds);

#endif
