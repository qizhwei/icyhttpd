#ifndef _DICT_H
#define _DICT_H

#include <stdint.h>

typedef int dict_bucket_t;
typedef struct dict_entry dict_entry_t;

typedef struct dict {
	uint32_t bucket_size;
	dict_bucket_t *buckets;
	uint32_t entry_size;
	uint32_t entry_used;
	dict_entry_t *entries;
	// entry_size - entry_used == number of entries in free_list
	dict_bucket_t free_list;
} dict_t;

extern void dict_init(dict_t *dict);
extern int dict_add_ptr(dict_t *dict, void *key, void *value);
extern int dict_remove_ptr(dict_t *dict, void *key);
extern int dict_get_ptr(dict_t *dict, void *key, void **value);
extern int dict_add_str(dict_t *dict, char *key, void *value);
extern int dict_remove_str(dict_t *dict, char *key);
extern int dict_get_str(dict_t *dict, char *key, void **value);
extern int dict_add_stri(dict_t *dict, char *key, void *value);
extern int dict_remove_stri(dict_t *dict, char *key);
extern int dict_get_stri(dict_t *dict, char *key, void **value);

#endif
