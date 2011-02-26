#ifndef _REQUEST_H
#define _REQUEST_H

#include "Types.h"
#include <vector>
#include <utility>

namespace Httpd
{
	class Request: NonCopyable, public Readable
	{
	public:
		Request(Readable &stream);
		virtual ~Request();
		virtual UInt32 Read(char *buffer, UInt32 size);

		const char *Method() { return &buffer[method]; }
		const char *URI() { return &buffer[uri]; }
		const char *QueryString() { return query == -1 ? nullptr : &buffer[query]; }
		const char *Host() { return host == -1 ? nullptr : &buffer[host]; }
		Int16 MajorVer() { return majorVer; }
		Int16 MinorVer() { return minorVer; }
	private:
		Readable &stream;
		std::vector<char> buffer;
		size_t begin;
		size_t end;
		std::vector<std::pair<Int16, Int16> > headers;
		Int16 method;
		Int16 uri;
		Int16 query;
		Int16 host;
		Int16 majorVer;
		Int16 minorVer;
	};
}

#endif
