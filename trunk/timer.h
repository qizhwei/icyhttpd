#ifndef _TIMER_H
#define _TIMER_H

#include "handle.h"
#include "semaphore.h"
#include "win32.h"

typedef struct timer {
	semaphore_t semaphore;
	HANDLE os_timer;
} timer_t;

extern int timer_init(handle_t timer);
extern void timer_uninit(handle_t timer);
extern int timer_set(handle_t timer, int milliseconds);

#endif
