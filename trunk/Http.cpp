#include "Http.h"
#include "Types.h"
#include "Constant.h"
#include "Exception.h"
#include "Socket.h"
#include "Win32.h"
#include "Utility.h"
#include <cstring>
#include <utility>
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

	const char *Weekdays[7] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
	};

	const char *Months[12] = {
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

	void URIDecode(char *first, char *&last)
	{
		char *cur = first;
		char state = 0;
		char escape;

		while (*first != '\0') {
			*cur = *first;
			if (state == 0) {
				state = (*cur == '%');
			} else {
				if (*cur >= '0' && *cur <= '9')
					escape = (escape << 4) | (*cur - '0');
				else if (*cur >= 'A' && *cur <= 'F')
					escape = (escape << 4) | (*cur - 'A' + 10);
				else if (*cur >= 'a' && *cur <= 'f')
					escape = (escape << 4) | (*cur - 'a' + 10);
				else
					state = 0;
				if (state == 2) {
					cur -= 2;
					*cur = escape;
				}
				state = (state & 1) << 1;
			}

			++cur;
			++first;
		}

		*(last = cur) = '\0';
	}

	void URIRewrite(char *first, char *&last)
	{
		char *cur = first;
		int state = 0;

		while (true) {
			if (*first == '\\')
				*first = '/';

			if (*first == '/' || *first == '\0') {
				if (state == 1) {
					cur -= 2;
				} else if (state == 2) {
					cur -= 3;
					while (*cur != '\0')
						if (*--cur == '/')
							break;
					if (*cur == '\0')
						++cur;
				}
				state = 0;
			} else if (*first == '.') {
				++state;
			} else {
				state = INT_MIN;
			}

			if ((*cur = *first) == '\0')
				break;
			++cur;
			++first;
		}

		last = cur;
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

				URIDecode(uri, uriLast);
				URIRewrite(uri, uriLast);

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

		if (this->majorVer == 1 && this->minorVer >= 1 && this->host == NullOffset)
			throw BadRequestException(true);
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

	void HttpResponse::AppendHeader(const char *name, UInt64 value)
	{
		size_t nameSize = strlen(name);
		char valueBuffer[24];
		char *valueStr = ParseUInt<24>(valueBuffer, value);
		size_t valueSize = valueBuffer + 24 - valueStr;
		char *colon = ": ", *crlf = "\r\n";
		this->buffer.reserve(nameSize + valueSize + 4);
		this->buffer.insert(this->buffer.end(), name, name + nameSize);
		this->buffer.insert(this->buffer.end(), colon, colon + 2);
		this->buffer.insert(this->buffer.end(), valueStr, valueStr + valueSize);
		this->buffer.insert(this->buffer.end(), crlf, crlf + 2);
	}

	void HttpResponse::EndHeader(UInt16 status, const char *reason, bool lengthProvided)
	{
		if (this->entity)
			return;

		char titleBuffer[48];
		wsprintfA(titleBuffer, "HTTP/1.1 %u %s\r\n", static_cast<unsigned int>(status), reason);

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

	void HttpResponse::TransmitFile(HANDLE hFile, UInt64 offset, UInt32 size)
	{
		this->socket.TransmitFile(hFile, offset, size);
	}
}
