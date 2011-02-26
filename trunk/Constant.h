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
	const ULONG_PTR OverlappedOperationKey = 1;

	// Seek methods
	const int SeekBegin = 0;
	const int SeekCurrent = 1;
	const int SeekEnd = 2;

	// Accept fiber count
	const int AcceptFiberCount = 4;
}

#endif
