#ifndef _HTTP_H
#define _HTTP_H

#include "Types.h"
#include "Socket.h"
#include <vector>
#include <utility>

namespace Httpd
{
	typedef std::pair<const char *, const char *> HttpHeader;
	typedef std::pair<UInt16, UInt16> HttpVersion;

	class HttpRequest: NonCopyable
	{
	public:
		HttpRequest(Socket &socket);
		void Flush();
		UInt32 Read(char *buffer, UInt32 size);

		const char *Method() { return &buffer[method]; }
		const char *URI() { return &buffer[uri]; }
		const char *Extension() { return ext == NullOffset ? "." : &buffer[ext]; };
		const char *QueryString() { return query == NullOffset ? nullptr : &buffer[query]; }
		const char *Host() { return host == NullOffset ? nullptr : &buffer[host]; }
		HttpVersion Version() { return HttpVersion(majorVer, minorVer); }
		Int64 ContentLength() { return contentLength; }
		bool Chunked() { return chunked; }
		bool KeepAlive() { return keepAlive; }
		size_t HeaderCount() { return headers.size(); }
		HttpHeader GetHeader(size_t index);

	private:
		Socket &socket;
		std::vector<char> buffer;
		size_t begin, end;
		std::vector<std::pair<UInt16, UInt16> > headers;
		UInt16 method, uri, ext, query, host;
		UInt16 majorVer, minorVer;
		Int64 contentLength;
		bool chunked;
		bool keepAlive;

		static const UInt16 NullOffset = 65535;
	};

	class HttpResponse: NonCopyable
	{
	public:
		HttpResponse(Socket &socket, HttpVersion requestVer);
		void AppendTitle(UInt16 status);
		void AppendHeader(HttpHeader header);
		void EndHeader();
		virtual void Write(const char *buffer, UInt32 size);
		// TODO: TransmitFile, dynamic_cast<Socket &> if bad_cast, simulate
		void Flush();
	private:
		Socket &socket;
		// TODO: ChunkedWriter
		std::vector<char> buffer;
		HttpVersion requestVer;
		UInt16 titleOffset;
		bool entity;
		bool chunked;

		static const size_t TitleSize = 48;
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
