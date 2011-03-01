#ifndef _UTILITY_H
#define _UTILITY_H

#include "Types.h"
#include "Exception.h"
#include "Win32.h"
#include <string>
#include <cstring>
#include <functional>
#include <cstddef>
#include "Dispatcher.h"
#include "Constant.h"

namespace
{
	volatile long PipeCount;
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

	int CreatePipePairDuplex(HANDLE hPipe[2])
	{
		const int BUFFER_SIZE = 4096; 
		const int PIPE_TIMEOUT = 1000; 
		wchar_t PipeName[48];
		
		wsprintf(PipeName, L"\\\\.\\pipe\\icyhttpd\\critter.%08x.%08x", GetCurrentProcessId(),  InterlockedIncrement(&PipeCount));

		hPipe[0] = CreateNamedPipe(PipeName, PIPE_ACCESS_DUPLEX 
				, PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, BUFFER_SIZE, BUFFER_SIZE 
				, PIPE_TIMEOUT,NULL);

		try {
			Dispatcher::Instance().BindHandle(hPipe, OverlappedOperationKey);
		} catch (...) {
			CloseHandle(hPipe);
			throw;
		}

		hPipe[1] = CreateFile(PipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
						OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		if (hPipe[1] == INVALID_HANDLE_VALUE) {
			CloseHandle(hPipe[0]);
			return 1;
		} 

		return 0;
	}
	
	int CreatePipePair(HANDLE hPipe[2])
	{
		const int BUFFER_SIZE = 4096; 
		const int PIPE_TIMEOUT = 1000; 
		wchar_t PipeName[48];
		
		wsprintf(PipeName, L"\\\\.\\pipe\\icyhttpd\\critter.%08x.%08x", GetCurrentProcessId(),  InterlockedIncrement(&PipeCount));

		hPipe[0] = CreateNamedPipe(PipeName, PIPE_ACCESS_OUTBOUND 
				, PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, BUFFER_SIZE, BUFFER_SIZE 
				, PIPE_TIMEOUT,NULL);

		try {
			Dispatcher::Instance().BindHandle(hPipe, OverlappedOperationKey);
		} catch (...) {
			CloseHandle(hPipe);
			throw;
		}

		hPipe[1] = CreateFile(PipeName, GENERIC_WRITE, 0, NULL,
						OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		if (hPipe[1] == INVALID_HANDLE_VALUE) {
			CloseHandle(hPipe[0]);
			return 1;
		} 

		return 0;
	}
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
