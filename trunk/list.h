#ifndef _LIST_H
#define _LIST_H

#include "runtime.h"
#include <stddef.h>

#define OFFSET_OF(type, member) ((size_t) &((type *)0)->member)
#define CONTAINER_OF(ptr, type, member) ((type *)((char *)(ptr) - OFFSET_OF(type, member)))

typedef struct list {
	struct list *next, *prev;
} list_t;

static inline void list_init(list_t *list)
{
	list->next = list;
	list->prev = list;
}

static inline NOFAIL int list_empty(list_t *list)
{
	return list->next == list;
}

static inline void list_push_back(list_t *list, list_t *entry)
{
	list_t *back = list->prev;
	entry->next = list;
	entry->prev = back;
	list->prev = entry;
	back->next = entry;
}

static inline void list_push_front(list_t *list, list_t *entry)
{
	list_t *front = list->next;
	entry->next = front;
	entry->prev = list;
	list->next = entry;
	front->prev = entry;
}

static inline void list_remove(list_t *entry)
{
	entry->prev->next = entry->next;
	entry->next->prev = entry->prev;
}

static inline NOFAIL list_t *list_pop_back(list_t *list)
{
	list_t *back = list->prev;
	list_remove(back);
	return back;
}

static inline NOFAIL list_t *list_pop_front(list_t *list)
{
	list_t *front = list->next;
	list_remove(front);
	return front;
}

#endif
