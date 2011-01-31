#ifndef _SHRIMP_H
#define _SHRIMP_H

#include "handle.h"

extern int shrimp_init(void);
extern void shrimp_loop(void);

typedef struct shrimp shrimp_t;
typedef void shrimp_proc_t(void *param);

extern handle_t shrimp_create(shrimp_proc_t *proc, void *param);
extern handle_t shrimp_current(void);
extern void shrimp_exit(void);
extern void shrimp_switch(handle_t shrimp);
extern void shrimp_ready(handle_t shrimp);
extern void shrimp_block(void);
extern handle_t shrimp_timer(int milliseconds);

// return value: wake reason
extern int shrimp_wait(int n, int m, ...);

// return value: os event handle
extern void * shrimp_share_event(shrimp_proc_t *callback, void *param);

// TODO: shrimp_alert

// wake reasons
#define WAKE_TIMEOUT (0)

#endif
