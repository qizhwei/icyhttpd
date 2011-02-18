#ifndef _DICT_H
#define _DICT_H

#include "mem.h"
#include <stdint.h>

typedef int dict_bucket_t;

typedef struct dict_entry {
	int next;
	void *key;
	void *value;
} dict_entry_t;

typedef struct dict {
	uint32_t bucket_size;
	dict_bucket_t *buckets;
	uint32_t entry_size;
	uint32_t entry_used;
	dict_entry_t *entries;
	// entry_size - entry_used == number of entries in free_list
	dict_bucket_t free_list;
} dict_t;

static inline void dict_init(dict_t *dict)
{
	dict->bucket_size = 0;
	dict->buckets = NULL;
	dict->entry_size = 0;
	dict->entry_used = 0;
	dict->entries = NULL;
	dict->free_list = -1;
}

static inline void dict_uninit(dict_t *dict)
{
	mem_free(dict->buckets);
	mem_free(dict->entries);
}

extern int dict_add_ptr(dict_t *dict, void *key, void *value);
extern void **dict_query_ptr(dict_t *dict, void *key, int remove);
extern int dict_add_str(dict_t *dict, char *key, void *value);
extern void **dict_query_str(dict_t *dict, char *key, int remove);
extern int dict_add_stri(dict_t *dict, char *key, void *value);
extern void **dict_query_stri(dict_t *dict, char *key, int remove);

typedef int dict_walk_callback_t(void *u, void *key, void *value);

static inline int dict_walk(dict_t *dict, dict_walk_callback_t *callback, void *param)
{
	int bucket, entry;

	for (bucket = 0; bucket != dict->bucket_size; ++bucket)
		for (entry = dict->buckets[bucket]; entry != -1; entry = dict->entries[entry].next)
			if (callback(param, dict->entries[entry].key, dict->entries[entry].value))
				return -1;

	return 0;
}

#endif
