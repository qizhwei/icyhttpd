#ifndef _CONSTANT_H
#define _CONSTANT_H

#include "Types.h"
#include "Win32.h"

namespace Httpd
{
	// Thread and fiber count
	const int DispatcherThreadCount = 4;
	const int AcceptFiberCount = 4;

	// Stack sizes
	const SIZE_T StackCommitSize = 4096;
	const SIZE_T StackReserveSize = 16384;

	// Buffer sizes
	const size_t MaxRequestBufferSize = 32768; // must be able to represent in UInt16
	const UInt32 BufferBlockSize = 4096;
	const UInt32 SocketProviderSize = 32;
}

#endif
