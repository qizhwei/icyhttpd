#ifndef _PROCESS_H
#define _PROCESS_H

extern int process_init(void);
extern void process_loop(void);

typedef struct process process_t;
typedef void proc_t(void *param);

typedef struct async {
	process_t *process;
} async_t;

extern int process_create(proc_t *proc, void *param);
extern process_t *process_current(void);
extern void process_exit(void);
extern void *process_share_event(proc_t *callback, void *param);
extern void process_block(async_t *async);
extern void process_unblock(async_t *async);
extern int process_timeout(int milliseconds, proc_t *abort_proc, void *param);

#endif
