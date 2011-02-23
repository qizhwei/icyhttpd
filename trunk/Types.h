#ifndef _TYPES_H
#define _TYPES_H

#include <cstdint>

namespace Httpd
{
	class NonCopyable
	{
	protected:
		NonCopyable() {}
		~NonCopyable() {}
	private:
		NonCopyable(const NonCopyable &);
		NonCopyable &operator=(const NonCopyable &);
	};

	class FatalException {};
	class ResourceInsufficientException {};

	typedef uint8_t Byte;
	typedef int32_t Int32;
	typedef uint32_t UInt32;
	typedef int64_t Int64;
	typedef uint64_t UInt64;

	typedef void Callback(void *);
}

#endif
