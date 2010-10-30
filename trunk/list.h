#ifndef _LIST_H
#define _LIST_H

/* this header implements a double linked list */

typedef struct _ListEntry {
	struct _ListEntry *next;
	struct _ListEntry *prev;
} ListEntry;

#define InitializeListHead(head) \
	do { \
		(head)->next = (head); \
		(head)->prev = (head); \
	} while (0)

#define InsertHeadList(head, entry) \
	do { \
		(entry)->prev = (head); \
		(entry)->next = (head)->next; \
		(head)->next->prev = (entry); \
		(head)->next = (entry); \
	} while (0)

#define InsertTailList(head, entry) \
	do { \
		(entry)->prev = (head)->prev; \
		(entry)->next = (head); \
		(head)->prev->next = (entry); \
		(head)->prev = (entry); \
	} while (0)

#define RemoveEntryList(entry) \
	do { \
		(entry)->prev->next = (entry)->next; \
		(entry)->next->prev = (entry)->prev; \
	} while (0)

#define IsListEmpty(head) \
	((head)->next == (head))

#endif
