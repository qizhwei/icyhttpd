#include "mem.h"
#include "win32.h"
#include <stddef.h>
#include <stdlib.h>

// TODO
// profile performance to determine whether or not
// to use the LFH policy provided by Windows Heap
// http://msdn.microsoft.com/en-us/library/aa366750.aspx

static HANDLE g_heap;

int mem_init(void)
{
	g_heap = GetProcessHeap();
	return 0;
}

void *mem_alloc(size_t size)
{
	return HeapAlloc(g_heap, 0, size);
}

void mem_free(void *p)
{
	HeapFree(g_heap, 0, p);
}
