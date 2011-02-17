#include "mem.h"
#include "win32.h"
#include "runtime.h"
#include <stddef.h>
#include <stdlib.h>

#define HEAP_LFH (2)

static HANDLE g_heap;

int mem_init(void)
{
	DWORD heap_info = HEAP_LFH;
	g_heap = GetProcessHeap();

	if (!HeapSetInformation(g_heap, HeapCompatibilityInformation, &heap_info, sizeof(heap_info)))
		runtime_ignore("Warning: LFH cannot be enabled.");

	return 0;
}

void *mem_alloc(size_t size)
{
	return HeapAlloc(g_heap, 0, size);
}

void mem_free(void *p)
{
	if (p != NULL)
		HeapFree(g_heap, 0, p);
}
