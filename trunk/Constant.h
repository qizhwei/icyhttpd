#ifndef _CONSTANT_H
#define _CONSTANT_H

#include "Types.h"
#include "Win32.h"

namespace Httpd
{
	// Dispatcher thread count
	const int ThreadCount = 4;

	// Fiber stack size information
	const SIZE_T StackCommitSize = 4096;
	const SIZE_T StackReserveSize = 16384;

	// Completion keys
	const ULONG_PTR FiberCreateKey = 0;
	const ULONG_PTR FiberDeleteKey = 1;
	const ULONG_PTR OverlappedOperationKey = 2;
}

#endif
