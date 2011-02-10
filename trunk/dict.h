#ifndef _DICT_H
#define _DICT_H

#include <stddef.h>

typedef int dict_bucket_t;
typedef struct dict_entry dict_entry_t;

typedef struct dict {
	size_t bucket_size;
	dict_bucket_t *buckets;
	size_t entry_size;
	size_t entry_used;
	dict_entry_t *entries;
	// entry_size - entry_used == number of entries in free_list
	dict_bucket_t free_list;
} dict_t;

extern void dict_init(dict_t *dict);
extern int dict_add_ptr(dict_t *dict, void *key, void *value);
extern int dict_query_ptr(dict_t *dict, void *key, void **value, int remove);
extern int dict_add_str(dict_t *dict, char *key, void *value);
extern int dict_query_str(dict_t *dict, char *key, void **value, int remove);
extern int dict_add_stri(dict_t *dict, char *key, void *value);
extern int dict_query_stri(dict_t *dict, char *key, void **value, int remove);

#endif
