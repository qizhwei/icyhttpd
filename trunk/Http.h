#ifndef _HTTP_H
#define _HTTP_H

#include "Types.h"
#include "Stream.h"
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
		HttpRequest(BufferedReader &reader);
		void Flush();
		UInt32 Read(char *buffer, UInt32 size);

		const char *Method() { return reader.BasePointer() + method; }
		const char *URI() { return reader.BasePointer() + uri; }
		const char *Extension() { return ext == NullOffset ? "." : reader.BasePointer() + ext; };
		const char *QueryString() { return query == NullOffset ? "" : reader.BasePointer() + query; }
		const char *Host() { return host == NullOffset ? "" : reader.BasePointer() + host; }
		HttpVersion Version() { return HttpVersion(majorVer, minorVer); }
		UInt64 RemainingLength() { return remainingLength; }
		bool Chunked() { return chunked; }
		bool KeepAlive() { return keepAlive; }
		UInt32 HeaderCount() { return headers.size(); }
		HttpHeader GetHeader(UInt32 index);

	private:
		BufferedReader &reader;
		std::vector<std::pair<UInt16, UInt16> > headers;
		UInt16 method, uri, ext, query, host;
		UInt16 majorVer, minorVer;
		UInt64 remainingLength;
		bool chunked;
		bool keepAlive;

		static const UInt16 NullOffset = UINT16_MAX;
	};

	class HttpResponse: NonCopyable
	{
	public:
		HttpResponse(Socket &socket, HttpVersion requestVersion, bool requestKeepAlive);
		void AppendHeader(const char *header);
		void EndHeader(UInt16 status, bool lengthProvided);
		void Write(const char *buffer, UInt32 size);
		void EndHeaderAndTransmitFile(HANDLE hFile);
		bool KeepAlive() { return keepAlive; }

	private:
		Socket &socket;
		bool assumeKeepAlive;
		bool entity;
		bool keepAlive;
		bool chunked;

		std::vector<char> buffer;
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
