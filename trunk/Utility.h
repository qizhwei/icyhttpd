#ifndef _UTILITY_H
#define _UTILITY_H

#include <string>
#include <cstring>
#include <functional>
#include <cstddef>

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
