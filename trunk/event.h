#ifndef _EVENT_H
#define _EVENT_H

#include "list.h"

typedef struct event {
	char manual_reset;
	char signaled;
	list_t wait_list;
} event_t;

extern void event_init(event_t *e, char manual_reset, char signaled);
extern void event_set(event_t *e);
extern void event_reset(event_t *e);
extern void event_wait(event_t *e);

#endif
