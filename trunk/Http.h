#ifndef _HTTP_H
#define _HTTP_H

#include "Types.h"
#include "BufferedWriter.h"
#include <vector>
#include <utility>

namespace Httpd
{
	typedef std::pair<const char *, const char *> HttpHeader;

	class HttpRequest: NonCopyable, public Readable
	{
	public:
		HttpRequest(Readable &stream);
		void Flush();
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
		HttpHeader GetHeader(size_t index);

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

	class HttpResponse: NonCopyable, public Writable
	{
	public:
		HttpResponse(Writable &stream, bool header, UInt16 status);
		void AppendHeader(const HttpHeader &header);
		void EndHeader();
		virtual void Write(const char *buffer, UInt32 size);
		void Flush();
	private:
		BufferedWriter writer;
	};

	class HttpUtility: NonCopyable
	{
	public:
		static HttpUtility &Instance();
		const char *ReasonPhrase(UInt16 status);
	private:
		HttpUtility();
		~HttpUtility();
	};
}

#endif
