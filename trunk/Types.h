#ifndef _TYPES_H
#define _TYPES_H

#include <cstdint>
#include <cstddef>
#include <climits>

namespace Httpd
{
	typedef int16_t Int16;
	typedef uint16_t UInt16;
	typedef int32_t Int32;
	typedef uint32_t UInt32;
	typedef int64_t Int64;
	typedef uint64_t UInt64;

	typedef void Callback(void *);
	typedef void BinaryCallback(void *, void *);

	class NonCopyable
	{
	protected:
		NonCopyable() {}
		~NonCopyable() {}
	private:
		NonCopyable(const NonCopyable &);
		NonCopyable &operator=(const NonCopyable &);
	};

	class POD
	{
	public:
		char *Buffer() { return reinterpret_cast<char *>(this); }
		const char *Buffer() const { return reinterpret_cast<const char *>(this); }
	};
}

#endif
