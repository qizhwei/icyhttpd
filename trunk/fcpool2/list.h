#ifndef _LIST_H
#define _LIST_H

// double linked-list implementation

typedef struct list {
	struct list *next;
	struct list *prev;
} list_t;

#define list_init(head) \
	do { \
		(head)->next = (head); \
		(head)->prev = (head); \
	} while (0)

#define list_add_head(head, entry) \
	do { \
		(entry)->prev = (head); \
		(entry)->next = (head)->next; \
		(head)->next->prev = (entry); \
		(head)->next = (entry); \
	} while (0)

#define list_add_tail(head, entry) \
	do { \
		(entry)->prev = (head)->prev; \
		(entry)->next = (head); \
		(head)->prev->next = (entry); \
		(head)->prev = (entry); \
	} while (0)

#define list_remove(entry) \
	do { \
		(entry)->prev->next = (entry)->next; \
		(entry)->next->prev = (entry)->prev; \
	} while (0)

#define list_is_empty(head) \
	((head)->next == (head))

#define container_of(entry, type, field) \
	((type *)((char *)entry - (size_t)(&((type *)0)->field)))

#endif
