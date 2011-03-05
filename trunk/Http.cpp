#include "Http.h"
#include "Types.h"
#include "Constant.h"
#include "Exception.h"
#include "Socket.h"
#include "Win32.h"
#include <cstring>
#include <utility>
#include <unordered_map>
#include <ctime>
#include <cassert>

using namespace Httpd;
using namespace std;

namespace
{
	const char *ParseCommaList(char *&next)
	{
		char *first = next, *last;
	
		if ((next = strchr(first, ',')) != nullptr) {
			last = next;
			*next++ = '\0';
		} else {
			last = first + strlen(first);
		}

		while (*first == ' ' || *first == '\t')
			++first;
		while (first != last && (last[-1] == ' ' || last[-1] == '\t'))
			*--last = '\0';

		return first;
	}

	UInt16 ParseUInt16(char *&p)
	{
		int i = 0;
		while (*p >= '0' && *p <= '9') {
			if ((i = i * 10 + (*p - '0')) > UINT16_MAX)
				i = UINT16_MAX;
			++p;
		}
		return i;
	}

	UInt64 ParseUInt64Dec(char *p)
	{
		while (*p == '0')
			++p;

		UInt64 u = 0;
		while (*p != '\0') {
			if (*p >= '0' && *p <= '9') {
				if (u > 1844674407370955161U)
					return UINT64_MAX;
				else if (u == 1844674407370955161U && *p > '5')
					return UINT64_MAX;
				u = u * 10 + (*p - '0');
			} else {
				return UINT64_MAX;
			}
			++p;
		}

		return u;
	}

	UInt64 ParseUInt64Hex(char *p)
	{
		while (*p == '0')
			++p;

		UInt16 u = 0;
		while (*p != '\0') {
			if (u > 0xfffffffffffffffU)
				return UINT64_MAX;
			else if (*p >= '0' && *p <= '9')
				u = (u << 4) | (*p - '0');
			else if (*p >= 'a' && *p <= 'f')
				u = (u << 4) | (*p - 'a' + 10);
			else if (*p >= 'A' && *p <= 'F')
				u = (u << 4) | (*p - 'A' + 10);
			else
				return UINT64_MAX;
			++p;
		}

		return u;
	}

	char *Weekdays[7] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
	};

	char *Months[12] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	};

	void GetCurrentDateBuffer(char dateBuffer[40])
	{
		__time64_t currentTime;
		tm currentTm;

		_time64(&currentTime);
		_gmtime64_s(&currentTm, &currentTime);
        wsprintfA(dateBuffer, "Date: %3s, %02d %3s %04d %02d:%02d:%02d GMT\r\n",
            Weekdays[currentTm.tm_wday], currentTm.tm_mday, Months[currentTm.tm_mon],
			currentTm.tm_year + 1900, currentTm.tm_hour, currentTm.tm_min, currentTm.tm_sec);
	}
}

namespace Httpd
{
	HttpRequest::HttpRequest(BufferedReader &reader)
		: reader(reader), remainingLength(0), chunked(false)
	{
		bool done = false;
		bool title = true;

		while (!done) {
			char *first = reader.ReadLine(true);
			char * const base = this->reader.BasePointer();
			if (first == nullptr)
				throw SystemException();

			if (title) {
				// Empty lines are ignored
				if (first[0] == '\0')
					continue;

				// Request method
				this->method = first - base;
				if ((first = strchr(first, ' ')) == nullptr)
					throw BadRequestException(true);
				*first++ = '\0';

				// Version
				char *uri = first;
				char *uriLast;
				if ((first = strchr(first, ' ')) == nullptr) {
					uriLast = uri + strlen(uri);
					this->majorVer = 0;
					this->minorVer = 9;
					this->keepAlive = false;

					// HTTP/0.9 requests don't have headers
					done = true;
				} else {
					uriLast = first;
					*first++ = '\0';
					if (strncmp(first, "HTTP/", 5))
						throw BadRequestException(true);
					first += 5;
					this->majorVer = ParseUInt16(first);
					if (*first++ != '.')
						throw BadRequestException(true);
					this->minorVer = ParseUInt16(first);
					if (*first != '\0')
						throw BadRequestException(true);

					if (this->majorVer != 1)
						throw HttpVersionNotSupportedException(true);
					if (this->minorVer == 0)
						this->keepAlive = false;
					else
						this->keepAlive = true;
				}

				// Request URI
				if (uri[0] == '/' || (uri[0] == '*' && uri[1] == '\0')) {
					this->host = NullOffset;
				} else {
					if (_strnicmp(uri, "http://", 7))
						throw BadRequestException(true);
					char *host = uri;
					if ((uri = strchr(uri + 7, '/')) == nullptr)
						uri = uriLast;
					memmove(host, host + 7, uri - host - 7);
					host[uri - host - 7] = '\0';
					if (uri == uriLast)
						*--uri = '/';
					uri[-1] = '\0';

					char *colon;
					if ((colon = strchr(host, ':')) != nullptr)
						*colon = '\0';
					this->host = host - base;
				}

				// Query string
				char *query;
				if ((query = strchr(uri, '?')) != nullptr) {
					uriLast = query;
					*query++ = '\0';
					this->query = query - base;
				} else {
					this->query = NullOffset;
				}

				// TODO: URI Decode and Rewrite dots and slashes in [uri, uriLast),
				// DO NOT FORGET to update uriLast

				// Extension
				assert(uri[-1] == '\0');
				char *ext = uriLast;
				while (true) {
					char c = *--ext;
					if (c == '.') {
						break;
					} else if (c == '/' || c == '\0') {
						ext = nullptr;
						break;
					}
				}

				this->uri = uri - base;
				this->ext = ext != nullptr ? ext - base : -1;

				title = false;
			} else {
				if (*first == '\0') {
					done = true;
				} else if (*first == ' ' || *first == '\t') {
					// Ignore the standard
					throw BadRequestException(true);
				} else {
					char *second = strchr(first, ':');
					if (second == nullptr)
						throw BadRequestException(true);
					*second++ = '\0';

					// Eat leading and trailing LWS
					while (*second == ' ' || *second == '\t')
						++second;

					// Filter out known headers
					if (this->host == NullOffset && !_stricmp(first, "Host")) {
						char *colon;
						if ((colon = strchr(second, ':')) != nullptr)
							*colon = '\0';
						this->host = second - base;
					} else if (!_stricmp(first, "Content-Length")) {
						if ((this->remainingLength = ParseUInt64Dec(second)) == UINT64_MAX)
							throw RequestEntityTooLargeException(true);
						this->chunked = false;
					} else if (!_stricmp(first, "Transfer-Encoding")) {
						if (_stricmp(second, "identity")) {
							this->remainingLength = 0;
							this->chunked = true;
						}
					} else if (!_stricmp(first, "Connection")) {
						do {
							const char *option = ParseCommaList(second);
							if (!_stricmp(option, "close"))
								this->keepAlive = false;
							else if (!_stricmp(option, "Keep-Alive"))
								this->keepAlive = true;
						} while (second != nullptr);
					} else {
						this->headers.push_back(std::pair<Int16, Int16>(first - base, second - base));
					}
				}
			}
		}
	}

	UInt32 HttpRequest::Read(char *buffer, UInt32 size)
	{
		if (remainingLength == 0) {
			if (chunked) {
				char *line;
				do
					line = this->reader.ReadLine();
				while (line[0] == '\0');
				if ((this->remainingLength = ParseUInt64Hex(line)) == UINT64_MAX)
					throw SystemException();
				if (remainingLength == 0) {
					this->chunked = false;
					return 0;
				}
			} else {
				return 0;
			}
		}

		if (remainingLength < static_cast<UInt64>(size))
			size = static_cast<UInt32>(remainingLength);
		if ((size = this->reader.Read(buffer, size)) == 0)
			throw SystemException();
		this->reader.Flush();
		remainingLength -= size;
		return size;
	}

	HttpHeader HttpRequest::GetHeader(UInt32 index)
	{
		std::pair<Int16, Int16> offsets = this->headers[index];
		const char *first = reader.BasePointer() + offsets.first;
		const char *second = reader.BasePointer() + offsets.second;
		return HttpHeader(first, second);
	}

	void HttpRequest::Flush()
	{
		char buffer[BufferBlockSize];
		while (this->Read(buffer, sizeof(buffer)) != 0);
		this->reader.Flush();
	}

	HttpResponse::HttpResponse(Socket &socket, HttpVersion requestVersion, bool requestKeepAlive)
		: socket(socket)
		, assumeKeepAlive(requestVersion.first == 1 && requestVersion.second >= 1)
		, entity(requestVersion.first == 0), keepAlive(requestKeepAlive)
		, chunked(requestVersion.first == 1 && requestVersion.second >= 1)
	{}

	void HttpResponse::AppendHeader(const char *header)
	{
		this->buffer.insert(this->buffer.end(), header, header + strlen(header));
	}

	void HttpResponse::EndHeader(UInt16 status, bool lengthProvided)
	{
		if (this->entity)
			return;

		char titleBuffer[48];
		wsprintfA(titleBuffer, "HTTP/1.1 %u %s\r\n",
			static_cast<unsigned int>(status), HttpUtility::Instance().ReasonPhrase(status));

		if (lengthProvided)
			this->chunked = false;
		else if (!this->chunked)
			this->keepAlive = false;

		// Server header
		AppendHeader("Server: icyhttpd/0.0\r\n");

		// Date header
		char dateBuffer[40];
		GetCurrentDateBuffer(dateBuffer);
		AppendHeader(dateBuffer);

		// Transfer encoding header
		if (this->chunked)
			AppendHeader("Transfer-Encoding: chunked\r\n");

		// Connection header
		if (!this->keepAlive)
			AppendHeader("Connection: close\r\n");
		else if (!this->assumeKeepAlive)
			AppendHeader("Connection: keep-alive\r\n");

		// Final CRLF
		const char *crlf = "\r\n";
		this->buffer.insert(this->buffer.end(), crlf, crlf + 2);

		// Write socket and clear buffer
		WSABUF WSABuf[2];
		WSABuf[0].buf = titleBuffer;
		WSABuf[0].len = strlen(titleBuffer);
		WSABuf[1].buf = &*this->buffer.begin();
		WSABuf[1].len = this->buffer.size();
		this->socket.Write(WSABuf, 2);
		vector<char>().swap(this->buffer);
		this->entity = true;
	}

	void HttpResponse::Write(const char *buffer, UInt32 size)
	{
		if (this->chunked) {
			char chunkedLength[12];
			wsprintfA(chunkedLength, "%x\r\n", size);
			WSABUF WSABuf[3];
			WSABuf[0].buf = chunkedLength;
			WSABuf[0].len = strlen(chunkedLength);
			WSABuf[1].buf = const_cast<char *>(buffer);
			WSABuf[1].len = size;
			WSABuf[2].buf = "\r\n";
			WSABuf[2].len = 2;
			this->socket.Write(WSABuf, 3);
		} else if (size != 0) {
			this->socket.Write(buffer, size);
		}
	}

	void HttpResponse::EndHeaderAndTransmitFile(HANDLE hFile)
	{
		throw NotImplementedException();
	}
}

namespace Httpd
{
	HttpUtility &HttpUtility::Instance()
	{
		static HttpUtility *u(new HttpUtility());
		return *u;
	}

	HttpUtility::HttpUtility()
	{
		reason.insert(make_pair(100, "Continue"));
		reason.insert(make_pair(101, "Switching Protocols"));
		reason.insert(make_pair(200, "OK"));
		reason.insert(make_pair(201, "Created"));
		reason.insert(make_pair(202, "Accepted"));
		reason.insert(make_pair(203, "Non-Authoritative Information"));
		reason.insert(make_pair(204, "No Content"));
		reason.insert(make_pair(205, "Reset Content"));
		reason.insert(make_pair(206, "Partial Content"));
		reason.insert(make_pair(300, "Multiple Choices"));
		reason.insert(make_pair(301, "Moved Permanently"));
		reason.insert(make_pair(302, "Found"));
		reason.insert(make_pair(303, "See Other"));
		reason.insert(make_pair(304, "Not Modified"));
		reason.insert(make_pair(305, "Use Proxy"));
		// 306 is unused and reserved
		reason.insert(make_pair(307, "Temporary Redirect"));
		reason.insert(make_pair(400, "Bad Request"));
		reason.insert(make_pair(401, "Unauthorized"));
		reason.insert(make_pair(402, "Payment Required"));
		reason.insert(make_pair(403, "Forbidden"));
		reason.insert(make_pair(404, "Not Found"));
		reason.insert(make_pair(405, "Method Not Allowed"));
		reason.insert(make_pair(406, "Not Acceptable"));
		reason.insert(make_pair(407, "Proxy Authentication Required"));
		reason.insert(make_pair(408, "Request Timeout"));
		reason.insert(make_pair(409, "Conflict"));
		reason.insert(make_pair(410, "Gone"));
		reason.insert(make_pair(411, "Length Required"));
		reason.insert(make_pair(412, "Precondition Failed"));
		reason.insert(make_pair(413, "Request Entity Too Large"));
		reason.insert(make_pair(414, "Request-URI Too Long"));
		reason.insert(make_pair(415, "Unsupported Media Type"));
		reason.insert(make_pair(416, "Request Range Not Satisfiable"));
		reason.insert(make_pair(417, "Expectation Failed"));
		reason.insert(make_pair(500, "Internal Server Error"));
		reason.insert(make_pair(501, "Not Implemented"));
		reason.insert(make_pair(502, "Bad Gateway"));
		reason.insert(make_pair(503, "Service Unavailable"));
		reason.insert(make_pair(504, "Gateway Timeout"));
		reason.insert(make_pair(505, "HTTP Version Not Supported"));
	}

	const char *HttpUtility::ReasonPhrase(UInt16 status)
	{
		auto i = reason.find(status);
		if (i == reason.end())
			throw SystemException();
		return i->second;
	}
}
