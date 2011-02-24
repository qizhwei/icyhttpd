#ifndef _TYPES_H
#define _TYPES_H

#include <cstdint>

namespace Httpd
{
	typedef int32_t Int32;
	typedef uint32_t UInt32;
	typedef int64_t Int64;
	typedef uint64_t UInt64;

	typedef void Callback(void *);

	class NonCopyable
	{
	protected:
		NonCopyable() {}
		~NonCopyable() {}
	private:
		NonCopyable(const NonCopyable &);
		NonCopyable &operator=(const NonCopyable &);
	};

	class Readable
	{
	public:
		virtual UInt32 Read(char *buffer, UInt32 size) = 0;
		virtual ~Readable() {};
	};

	class Writable
	{
	public:
		virtual void Write(char *buffer, UInt32 size) = 0;
		virtual ~Writable() {};
	};
}

#endif
