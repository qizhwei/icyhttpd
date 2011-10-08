#ifndef _TIMER_H
#define _TIMER_H

#include "types.h"

typedef struct timer timer_t;

extern int timer_startup(void);
extern timer_t *timer_create(int interval, callback_t *cb, void *u);
extern int timer_start(timer_t *t);
extern int timer_stop(timer_t *t);

#endif
