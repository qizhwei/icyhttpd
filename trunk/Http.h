#ifndef _HTTP_H
#define _HTTP_H

#include "Types.h"
#include "Socket.h"
#include <vector>
#include <utility>
#include <unordered_map>

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
		const char *QueryString() { return query == NullOffset ? "" : &buffer[query]; }
		const char *Host() { return host == NullOffset ? "" : &buffer[host]; }
		HttpVersion Version() { return HttpVersion(majorVer, minorVer); }
		UInt64 ContentLength() { return contentLength; }
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
		UInt64 contentLength;
		bool chunked;
		bool keepAlive;

		static const UInt16 NullOffset = 65535;
	};

	class HttpResponse: NonCopyable
	{
	public:
		HttpResponse(Socket &socket, HttpVersion requestVersion, bool requestKeepAlive);
		void AppendTitle(UInt16 status);
		void AppendHeader(HttpHeader header);
		void EndHeader(bool lengthProvided);
		void Write(const char *buffer, UInt32 size);
		// TODO: TransmitFile
		void Flush();
		bool KeepAlive() { return keepAlive; }

	private:
		Socket &socket;
		bool assumeKeepAlive;
		bool entity;
		bool keepAlive;
		bool chunked;

		std::vector<char> buffer;
		UInt16 titleOffset;

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
		std::unordered_map<UInt16, const char *> reason;
	};
}

#endif
