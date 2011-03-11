#ifndef _HTTP_H
#define _HTTP_H

#include "Types.h"
#include "Win32.h"
#include "Stream.h"
#include <vector>
#include <utility>

namespace Httpd
{
	class BufferedReader;
	class Socket;

	typedef std::pair<UInt16, UInt16> HttpVersion;

	class HttpRequest: NonCopyable
	{
	public:
		HttpRequest(BufferedReader &reader);
		UInt32 Read(char *buffer, UInt32 size);
		void Flush();

		// Header fields
		const char *Method();
		const char *URI();
		const char *Extension();
		const char *QueryString();
		const char *Host();
		const char *ContentLength();

		// Status
		HttpVersion Version();
		UInt64 RemainingLength();
		bool Chunked();
		bool KeepAlive();

	private:
		BufferedReader &reader;
		UInt16 method, uri, ext, query, host, contentLength;
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
		void AppendHeader(const char *name, UInt64 value);
		void EndHeader(UInt16 status, const char *reason, bool lengthProvided);
		void Write(const char *buffer, UInt32 size);
		void TransmitFile(HANDLE hFile, UInt64 offset, UInt32 size);
		bool KeepAlive() { return keepAlive; }

	private:
		Socket &socket;
		bool assumeKeepAlive;
		bool entity;
		bool keepAlive;
		bool chunked;

		std::vector<char> buffer;
	};
}

#endif
