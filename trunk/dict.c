#include "dict.h"
#include "mem.h"
#include <string.h>
#include <assert.h>

#define MIN_BUCKETS (8)
#define MIN_ENTRIES (MIN_BUCKETS * 2)

typedef int hash_func_t(void *key);

struct dict_entry {
	int next;
	void *key;
	void *value;
};

void dict_init(dict_t *dict)
{
	dict->bucket_size = 0;
	dict->buckets = NULL;
	dict->entry_size = 0;
	dict->entry_used = 0;
	dict->entries = NULL;
	dict->free_list = -1;
}

static inline int expand_entries(dict_t *dict)
{
	size_t new_size = dict->entry_size ? dict->entry_size * 2 : MIN_ENTRIES;
	dict_entry_t *entries = mem_alloc(sizeof(dict_entry_t) * new_size);
	int entry;

	if (entries == NULL)
		return -1;

	// copy original entries
	for (entry = 0; entry != dict->entry_size; ++entry)
		entries[entry] = dict->entries[entry];

	// link new entries
	assert(dict->free_list == -1);
	dict->free_list = entry;
	for (; entry != new_size - 1; ++entry)
		entries[entry].next = entry + 1;
	entries[entry].next = -1;

	mem_free(dict->entries);
	dict->entries = entries;
	dict->entry_size = new_size;
	return 0;
}

static inline int expand_buckets(dict_t *dict, hash_func_t *hash_func)
{
	size_t new_size = dict->bucket_size ? dict->bucket_size * 2 : MIN_BUCKETS;
	dict_bucket_t *buckets = mem_alloc(sizeof(dict_bucket_t) * new_size);
	int bucket, entry, next_entry, new_bucket;

	if (buckets == NULL)
		return -1;

	// initialize new buckets
	for (bucket = 0; bucket != new_size; ++bucket)
		buckets[bucket] = -1;

	// rehash original buckets
	for (bucket = 0; bucket != dict->bucket_size; ++bucket) {
		for (entry = dict->buckets[bucket]; entry != -1; entry = next_entry) {
			next_entry = dict->entries[entry].next;
			new_bucket = hash_func(dict->entries[entry].key) & (new_size - 1);
			dict->entries[entry].next = buckets[new_bucket];
			buckets[new_bucket] = entry;
		}
	}

	mem_free(dict->buckets);
	dict->buckets = buckets;
	dict->bucket_size = new_size;
	return 0;
}

static inline int alloc_entry(dict_t *dict)
{
	int entry;

	if (dict->free_list == -1) {
		if (expand_entries(dict))
			return -1;
	}

	++dict->entry_used;
	entry = dict->free_list;
	dict->free_list = dict->entries[entry].next;
	return entry;
}

static inline void free_entry(dict_t *dict, int entry)
{
	--dict->entry_used;
	dict->entries[entry].next = dict->free_list;
	dict->free_list = entry;

	// TODO: reduce entries if too many items in free_list
}

static inline int dict_add(dict_t *dict, void *key, void *value, hash_func_t *hash_func)
{
	int entry = alloc_entry(dict);
	int bucket;

	if (entry == -1)
		return -1;

	if (dict->entry_used > dict->bucket_size * 2) {
		if (expand_buckets(dict, hash_func)) {
			free_entry(dict, entry);
			return -1;
		}
	}

	bucket = hash_func(key) & (dict->bucket_size - 1);
	dict->entries[entry].next = dict->buckets[bucket];
	dict->entries[entry].key = key;
	dict->entries[entry].value = value;
	dict->buckets[bucket] = entry;
	return 0;
}

static inline void *dict_query(dict_t *dict, void *key, int remove, hash_func_t *hash_func)
{
	// TODO
}

static inline int ptr_hash(void *ptr)
{
	// TODO
	return 0;
}

int dict_add_ptr(dict_t *dict, void *key, void *value)
{
	return dict_add(dict, key, value, ptr_hash);
}

void *dict_query_ptr(dict_t *dict, void *key, int remove)
{
	return dict_query(dict, key, remove, ptr_hash);
}
