#ifndef _UTILITY_H
#define _UTILITY_H

#include "Types.h"
#include "Win32.h"
#include "Dispatcher.h"
#include <string>
#include <cstring>
#include <functional>

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
		Win32Handle()
			: hObject(NULL)
		{}

		Win32Handle(HANDLE hObject)
			: hObject(hObject)
		{}

		Win32Handle(Win32Handle &&h)
			: hObject(h.hObject)
		{
			h.hObject = NULL;
		}

		~Win32Handle()
		{
			this->Close();
		}

		void Handle(HANDLE hObject)
		{
			this->Close();
			this->hObject = hObject;
		}

		HANDLE Handle() const
		{
			return hObject;
		}

		HANDLE Release()
		{
			HANDLE hObject = this->hObject;
			this->hObject = NULL;
			return hObject;
		}

		void Close()
		{
			if (hObject != NULL) {
				CloseHandle(hObject);
				hObject = NULL;
			}
		}
	private:
		HANDLE hObject;
	};

	extern std::pair<Win32Handle, Win32Handle> CreatePipePairDuplex();
	extern std::pair<Win32Handle, Win32Handle> CreatePipePair();
	extern HANDLE OpenFile(const wchar_t *path);
	extern UInt64 GetFileSize(HANDLE hFile);

	extern std::wstring MB2WC(UINT codePage, const std::string &s);
	extern char *ParseCommaList(char *&next);
	extern UInt64 ParseUInt64Dec(char *p);

	inline void EatLWS(char *&p)
	{
		while (*p == ' ' || *p == '\t')
			++p;
	}

	inline void EatLWSBackwards(char *first, char *&last)
	{
		while (first != last && (last[-1] == ' ' || last[-1] == '\t'))
			*--last = '\0';
	}

	inline void EatLWSBackwardsUnsafe(char *&last)
	{
		while (last[-1] == ' ' || last[-1] == '\t')
			*--last = '\0';
	}

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

	class Completion: NonCopyable
	{
	public:
		Completion()
			: lpFiber(NULL)
		{}

		Completion(LPVOID lpFiber)
			: lpFiber(lpFiber)
		{}

		void SwitchBack()
		{
			SwitchToFiber(this->lpFiber);
		}

		virtual bool operator()() { return true; }

	protected:
		void Release()
		{
			DeleteFiber(this->lpFiber);
		}

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
			: Completion(GetCurrentFiber())
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
