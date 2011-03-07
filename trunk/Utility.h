#ifndef _UTILITY_H
#define _UTILITY_H

#include "Types.h"
#include "Win32.h"
#include "Dispatcher.h"
#include <string>
#include <cstring>
#include <functional>
#include <queue>
#include <cassert>

namespace
{
	inline char CiIsLower(char c)
	{
		return c >= 'a' && c <= 'z';
	}

	inline char CiToUpper(char c)
	{
		return CiIsLower(c) ? c + ('A' - 'a') : c;
	}
}

namespace Httpd
{
	struct CiCharTraits: public std::char_traits<char>
	{
		static bool eq(char c1, char c2) { return CiToUpper(c1) == CiToUpper(c2); }
		static bool ne(char c1, char c2) { return CiToUpper(c1) != CiToUpper(c2); }
		static bool lt(char c1, char c2) { return CiToUpper(c1) <  CiToUpper(c2); }
		static int compare(const char *s1, const char *s2, size_t n) { return _memicmp(s1, s2, n); }
		static const char *find(const char *s, int n, char a) { while (n-- > 0 && CiToUpper(*s) != CiToUpper(a)) { ++s; } return s; }
	};

	typedef std::basic_string<char, CiCharTraits> CiString;

	class Win32Handle: NonCopyable
	{
	public:
		Win32Handle(HANDLE hObject)
			: hObject(hObject)
		{}

		~Win32Handle()
		{
			CloseHandle(hObject);
		}

		HANDLE Handle() const
		{
			return hObject;
		}
	private:
		HANDLE hObject;
	};

	extern void CreatePipePairDuplex(HANDLE hPipe[2]);
	extern void CreatePipePair(HANDLE hPipe[2]);
	extern HANDLE OpenFile(const wchar_t *path);
	extern UInt64 GetFileSize(HANDLE hFile);

	template<size_t BufferSize, typename UIntType>
	char *ParseUInt(char buffer[BufferSize], UIntType i)
	{
		char *p = buffer + BufferSize;
		
		if (i == 0) {
			*--p = '0';
		} else {
			do {
				*--p = '0' + i % 10;
				i /= 10;
			} while (i != 0);
		}

		return p;
	}

	class Completion
	{
	public:
		Completion()
		{}

		Completion(LPVOID lpFiber)
			: lpFiber(lpFiber)
		{}

		void SwitchBack()
		{
			SwitchToFiber(this->lpFiber);
		}

		virtual bool operator()() { return true; }
	private:
		LPVOID lpFiber;
	};

	class OverlappedCompletion: public OVERLAPPED, public Completion
	{
	public:
		OverlappedCompletion()
			: Completion(GetCurrentFiber())
		{
			std::memset(static_cast<OVERLAPPED *>(this), 0, sizeof(OVERLAPPED));
		}

		OverlappedCompletion(UInt64 offset)
		{
			this->OVERLAPPED::Internal = 0;
			this->OVERLAPPED::InternalHigh = 0;
			this->OVERLAPPED::Offset = reinterpret_cast<LARGE_INTEGER *>(&offset)->LowPart;
			this->OVERLAPPED::OffsetHigh = reinterpret_cast<LARGE_INTEGER *>(&offset)->HighPart;
			this->OVERLAPPED::hEvent = NULL;
		}
	};

	template<typename LockType>
	class Lock
	{
	public:
		Lock(LockType &lock)
			: lock(lock)
		{
			lock.Lock();
		}

		~Lock()
		{
			lock.Unlock();
		}

	private:
		LockType &lock;
	};

	class ThreadLock: NonCopyable
	{
	public:
		ThreadLock()
		{
			InitializeCriticalSection(&cs);
		}

		~ThreadLock()
		{
			DeleteCriticalSection(&cs);
		}

		void Lock()
		{
			EnterCriticalSection(&cs);
		}

		void Unlock()
		{
			LeaveCriticalSection(&cs);
		}

	private:
		CRITICAL_SECTION cs;
	};

	template<typename Type>
	class Shared
	{
	public:
		Shared()
			: counter(1)
		{}

		~Shared()
		{
			assert(counter == 0);
		}

		Type *AddRef()
		{
			InterlockedIncrement(&counter);
			return static_cast<Type *>(this);
		}

		void Release()
		{
			if (InterlockedDecrement(&counter) == 0)
				delete static_cast<Type *>(this);
		}

	private:
		volatile LONG counter;
	};
}

namespace std
{
	template<>
	class hash<Httpd::CiString>: public std::unary_function<Httpd::CiString, size_t>
	{
	public:
		size_t operator()(const Httpd::CiString &key) const
		{
			size_t val = 2166136261U;
			size_t first = 0;
			size_t last = key.size();
			size_t stride = 1 + last / 10;

			for(; first < last; first += stride)
				val = 16777619U * val ^ (size_t)CiToUpper(key[first]);
			return (val);
		}
	};
}

#endif
