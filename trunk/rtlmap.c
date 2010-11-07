#include "rtl.h"
#include <ctype.h>
#include <assert.h>

#define INITIAL_SLOTS (1)
#define OFFSET_BASIS (2166136261U)

static size_t RtlpStringHash(void *string)
{
	char *p = string;
	size_t hash = OFFSET_BASIS;
	
	while (*p) {
		hash ^= (size_t)*p;
		hash += (hash<<1) + (hash<<4) + (hash<<7) + (hash<<8) + (hash<<24);
		p += 1;
	}

	return hash;
}

static size_t RtlpStringHashInsensitive(void *string)
{
	char *p = string;
	size_t hash = OFFSET_BASIS;
	
	while (*p) {
		hash ^= (size_t)tolower(*p);
		hash += (hash<<1) + (hash<<4) + (hash<<7) + (hash<<8) + (hash<<24);
		p += 1;
	}

	return hash;
}

static int RtlpStringEqual(void *string0, void *string1)
{
	return strcmp(string0, string1);
}

static int RtlpStringEqualInsensitive(void *string0, void *string1)
{
	return stricmp(string0, string1);
}

RtlMap * RtlpCreateMap(RtlHashFunc *hash, RtlEqualFunc *equal, size_t slots)
{
	RtlMap *map = RtlAllocateHeap(sizeof(RtlMap) + INITIAL_SLOTS * sizeof(RtlMapRecord *));
	int index;
	
	if (map == NULL) {
		return NULL;
	}
	
	map->Hash = hash;
	map->Equal = equal;
	map->SlotCount = slots;
	map->ElementCount = 0;
	
	for (index = 0; index < slots; ++index) {
		map->Slot[index] = NULL;
	}

	return map;
}

RtlMap * RtlpResizeMap(RtlMap *map, size_t slots)
{
	RtlMap *result = RtlpCreateMap(map->Hash, map->Equal, slots);

	if (result != NULL) {
		RtlFreeHeap(map);
		return result;
	} else {
		return map;
	}
}

RtlMap * RtlCreateMap(RtlHashFunc *hash, RtlEqualFunc *equal)
{
	return RtlpCreateMap(hash, equal, INITIAL_SLOTS);
}

RtlMap * RtlCreateStringMap(int caseInsensitive)
{
	if (!caseInsensitive) {
		return RtlCreateMap(&RtlpStringHash, &RtlpStringEqual);
	} else {
		return RtlCreateMap(&RtlpStringHashInsensitive, &RtlpStringEqualInsensitive);
	}
}

static inline RtlMapRecord ** RtlpFindSlot(RtlMap *map, void *key)
{
	return &map->Slot[map->Hash(key) % map->SlotCount];
}

static void RtlpHashRecord(RtlMap *map, RtlMapRecord *record)
{
	RtlMapRecord *current;
	RtlMapRecord **slot;
	
	while (record != NULL) {
		current = record;
		record = record->Next;
		slot = RtlpFindSlot(map, current->Key);
		current->Next = *slot;
		*slot = current;
		map->ElementCount += 1;
	}
}

static RtlMapRecord * RtlpConvertMapToList(RtlMap *map)
{
	RtlMapRecord head = {
		.Next = NULL,
	};
	RtlMapRecord *list = &head;
	size_t elementCount = 0;
	size_t slotIndex = 0;
	
	while (slotIndex < map->SlotCount) {
		list->Next = map->Slot[slotIndex];
		while (list->Next != NULL) {
			list = list->Next;
			elementCount += 1;
		}
		map->Slot[slotIndex] = NULL;
		slotIndex += 1;
	}
	
	assert(elementCount == map->ElementCount);
	map->ElementCount = 0;
	return head.Next;
}

int RtlAddMap(RtlMap **map, void *key, void *value)
{
	RtlMapRecord *record = RtlAllocateHeap(sizeof(RtlMapRecord));
	if (record == NULL) {
		return 1;
	}
	
	record->Key = key;
	record->Value = value;
	
	if ((**map).ElementCount < (**map).SlotCount * 2) {
		record->Next = NULL;
	} else {
		record->Next = RtlpConvertMapToList(*map);
		assert((**map).ElementCount == 0);
		*map = RtlpResizeMap(*map, (**map).SlotCount * 2);
	}
	
	RtlpHashRecord(*map, record);
	return 0;
}

int RtlQueryMap(RtlMap **map, void *key, void **value, int remove)
{
	RtlMapRecord **record = RtlpFindSlot(*map, key);
	RtlMapRecord *current;
	
	while ((current = *record) != NULL) {
		if (!(**map).Equal(key, current->Key)) {
			if (value != NULL) {
				*value = current->Value;
			}
			if (remove != 0) {
				*record = current->Next;
				RtlFreeHeap(current);
				(**map).ElementCount -= 1;
				
				if ((**map).SlotCount > INITIAL_SLOTS &&
					(**map).ElementCount * 2 < (**map).SlotCount)
				{
					current = RtlpConvertMapToList(*map);
					assert((**map).ElementCount == 0);
					*map = RtlpResizeMap(*map, (**map).SlotCount / 2);
					RtlpHashRecord(*map, current);
				}
			}
			return 0;
		}
		record = &current->Next;
	}
	
	return 1;
}
