#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

#include "handle.h"
#include "list.h"
#include <stdarg.h>

typedef struct semaphore {
	int available;
	list_t queue;
} semaphore_t;

#define MAX_WAIT_SEMAPHORES (8)

extern void semaphore_init(handle_t semaphore);
extern void semaphore_signal(handle_t semaphore);
extern int semaphore_wait(int n, int m, va_list vl);

#endif
