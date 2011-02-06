#ifndef _PROCESS_H
#define _PROCESS_H

#include "handle.h"

extern int process_init(void);
extern void process_loop(void);

typedef struct process process_t;
typedef void process_proc_t(void *param);

extern handle_t process_create(process_proc_t *proc, void *param);
extern handle_t process_current(void);
extern void process_exit(void);
extern void process_switch(handle_t process);
extern void process_ready(handle_t process);
extern void process_block(void);
extern handle_t process_timer(int milliseconds);

// return value: wake reason
extern int process_wait(int n, int m, ...);

// return value: os event handle
extern void * process_share_event(process_proc_t *callback, void *param);

// wake reasons
#define WAKE_TIMEOUT (0)

#endif
