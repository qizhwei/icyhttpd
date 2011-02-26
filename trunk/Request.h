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
		typedef std::pair<const char *, const char *> Header;

		Request(Readable &stream);
		virtual UInt32 Read(char *buffer, UInt32 size);

		const char *Method() { return &buffer[method]; }
		const char *URI() { return &buffer[uri]; }
		const char *QueryString() { return query == -1 ? nullptr : &buffer[query]; }
		const char *Host() { return host == -1 ? nullptr : &buffer[host]; }
		Int16 MajorVer() { return majorVer; }
		Int16 MinorVer() { return minorVer; }
		Int64 ContentLength() { return contentLength; }
		bool Chunked() { return chunked; }
		bool KeepAlive() { return keepAlive; }
		size_t HeaderCount() { return headers.size(); }
		Header GetHeader(size_t index);

	private:
		Readable &stream;
		std::vector<char> buffer;
		size_t begin, end;
		std::vector<std::pair<Int16, Int16> > headers;
		Int16 method, uri, query, host;
		Int16 majorVer, minorVer;
		Int64 contentLength;
		bool chunked;
		bool keepAlive;
	};
}

#endif
