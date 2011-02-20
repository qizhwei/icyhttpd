#include "dict.h"
#include "mem.h"
#include "runtime.h"
#include <stdint.h>
#include <string.h>

#define MIN_ENTRIES (16)
#define MIN_BUCKETS (17)

typedef uint32_t hash_func_t(void *key);
typedef int equal_func_t(void *key0, void *key1);

static uint32_t primes[] = {
	17, 37, 67, 131, 257, 521, 1031, 2053,
	4099, 8209, 16411, 32771, 65537, 131101, 262147, 524309,
	1048583, 2097169, 4194319, 8388617, 16777259, 33554467, 67108879, 134217757,
	268435459, 536870923, 1073741827, 2147483659U
};

static inline NOFAIL uint32_t next_prime(uint32_t m)
{
	uint32_t *p = primes;

	while (*p <= m)
		++p;

	return *p;
}

static inline NOFAIL uint32_t prev_prime(uint32_t m)
{
	uint32_t *p = primes + 1;

	while (*p < m)
		++p;

	return *--p;
}

static inline void expand_entries(dict_t *dict)
{
	uint32_t new_size = dict->entry_size ? (dict->entry_size << 1) : MIN_ENTRIES;
	dict_entry_t *entries = mem_alloc(sizeof(dict_entry_t) * new_size);
	int entry;

	// copy original entries
	for (entry = 0; entry != dict->entry_size; ++entry)
		entries[entry] = dict->entries[entry];

	// link new entries
	dict->free_list = entry;
	for (; entry != new_size - 1; ++entry)
		entries[entry].next = entry + 1;
	entries[entry].next = -1;

	mem_free(dict->entries);
	dict->entries = entries;
	dict->entry_size = new_size;
}

static inline void reduce_entries(dict_t *dict)
{
	uint32_t new_size = dict->entry_size >> 1;
	dict_entry_t *entries = mem_alloc(sizeof(dict_entry_t) * new_size);
	int bucket, old_entry, entry, *entry_ptr;

	// copy entries by iterate through buckets
	entry = 0;
	for (bucket = 0; bucket != dict->bucket_size; ++bucket) {
		entry_ptr = &dict->buckets[bucket];
		for (old_entry = *entry_ptr; old_entry != -1; old_entry = dict->entries[old_entry].next) {
			*entry_ptr = entry;
			entry_ptr = &entries[entry].next;
			entries[entry].key = dict->entries[old_entry].key;
			entries[entry].value = dict->entries[old_entry].value;
			++entry;
		}
		*entry_ptr = -1;
	}

	// rebuild free list
	dict->free_list = entry;
	for (; entry != new_size - 1; ++entry)
		entries[entry].next = entry + 1;
	entries[entry].next = -1;

	mem_free(dict->entries);
	dict->entries = entries;
	dict->entry_size = new_size;
}

static inline void resize_buckets(dict_t *dict, uint32_t new_size, hash_func_t *hash_func)
{
	dict_bucket_t *buckets = mem_alloc(sizeof(dict_bucket_t) * new_size);
	int bucket, entry, next_entry, new_bucket;

	// initialize new buckets
	for (bucket = 0; bucket != new_size; ++bucket)
		buckets[bucket] = -1;

	// rehash original buckets
	for (bucket = 0; bucket != dict->bucket_size; ++bucket) {
		for (entry = dict->buckets[bucket]; entry != -1; entry = next_entry) {
			next_entry = dict->entries[entry].next;
			new_bucket = hash_func(dict->entries[entry].key) % new_size;
			dict->entries[entry].next = buckets[new_bucket];
			buckets[new_bucket] = entry;
		}
	}

	mem_free(dict->buckets);
	dict->buckets = buckets;
	dict->bucket_size = new_size;
}

static inline NOFAIL int alloc_entry(dict_t *dict)
{
	int entry;

	if (dict->free_list == -1)
		expand_entries(dict);

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

	if (dict->entry_size > MIN_ENTRIES
		&& dict->entry_used < (dict->entry_size >> 2))
		reduce_entries(dict);
}

static inline void dict_add(dict_t *dict, void *key, void *value, hash_func_t *hash_func)
{
	int entry = alloc_entry(dict);
	int bucket;
	uint32_t new_size;

	if (dict->entry_used >= dict->bucket_size) {
		new_size = next_prime(dict->bucket_size);
		resize_buckets(dict, new_size, hash_func);
	}

	bucket = hash_func(key) % dict->bucket_size;
	dict->entries[entry].next = dict->buckets[bucket];
	dict->entries[entry].key = key;
	dict->entries[entry].value = value;
	dict->buckets[bucket] = entry;
}

static inline MAYFAIL(NULL) void **dict_query(dict_t *dict, void *key,
	 int remove, hash_func_t *hash_func, equal_func_t *equal_func)
{
	void **result;
	int bucket, *entry_ptr, entry;

	if (!dict->bucket_size)
		return NULL;

	bucket = hash_func(key) % dict->bucket_size;
	entry_ptr = &dict->buckets[bucket];
	entry = *entry_ptr;
	while (entry != -1) {
		if (!equal_func(key, dict->entries[entry].key)) {
			result = &dict->entries[entry].value;
			if (remove) {
				*entry_ptr = dict->entries[entry].next;
				free_entry(dict, entry);

				if (dict->bucket_size > MIN_BUCKETS
					&& (dict->entry_used << 2) < dict->bucket_size)
					resize_buckets(dict, prev_prime(dict->bucket_size), hash_func);
			}

			return result;
		}

		entry_ptr = &dict->entries[entry].next;
		entry = *entry_ptr;
	}

	return NULL;
}

#define GOLDEN (2654435761U)
static inline NOFAIL uint32_t ptr_hash(void *ptr)
{
	return (uint32_t)ptr * GOLDEN;
}

static inline NOFAIL int ptr_equal(void *ptr0, void *ptr1)
{
	return !(ptr0 == ptr1);
}

#define OFFSET_BASIS (2166136261U)
static inline NOFAIL uint32_t str_hash(void *str)
{
	char *p = str;
	uint32_t hash = OFFSET_BASIS;

	while (*p) {
		hash ^= (uint32_t)*p++;
		hash += (hash<<1) + (hash<<4) + (hash<<7) + (hash<<8) + (hash<<24);
	}

	return hash;
}

static inline NOFAIL int str_equal(void *str0, void *str1)
{
	return strcmp(str0, str1);
}

void dict_add_ptr(dict_t *dict, void *key, void *value)
{
	dict_add(dict, key, value, ptr_hash);
}

MAYFAIL(NULL) void **dict_query_ptr(dict_t *dict, void *key, int remove)
{
	return dict_query(dict, key, remove, ptr_hash, ptr_equal);
}

void dict_add_str(dict_t *dict, char *key, void *value)
{
	dict_add(dict, key, value, str_hash);
}

MAYFAIL(NULL) void **dict_query_str(dict_t *dict, char *key, int remove)
{
	return dict_query(dict, key, remove, str_hash, str_equal);
}
