#include "Http.h"
#include "Types.h"
#include "Constant.h"
#include "Exception.h"
#include "Socket.h"
#include "Win32.h"
#include "Utility.h"
#include "Stream.h"
#include <cstring>
#include <utility>
#include <ctime>
#include <cassert>
#include <algorithm>

using namespace Httpd;
using namespace std;

namespace
{
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

	void GetCurrentDateBuffer(char dateBuffer[32])
	{
		__time64_t currentTime;
		tm currentTm;

		_time64(&currentTime);
		_gmtime64_s(&currentTm, &currentTime);
        wsprintfA(dateBuffer, "%3s, %02d %3s %04d %02d:%02d:%02d GMT",
            Weekdays[currentTm.tm_wday], currentTm.tm_mday, Months[currentTm.tm_mon],
			(currentTm.tm_year + 1900) % 10000, currentTm.tm_hour, currentTm.tm_min, currentTm.tm_sec);
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

	class HeaderComparePredicate
	{
	public:
		HeaderComparePredicate(char *base)
			: base(base)
		{}

		bool operator()(const char *a, pair<UInt16, UInt16> b)
		{
			return (*this)(a, this->base + b.first);
		}

		bool operator()(pair<UInt16, UInt16> a, const char *b)
		{
			return (*this)(this->base + a.first, b);
		}

		bool operator()(pair<UInt16, UInt16> a, pair<UInt16, UInt16> b)
		{
			return (*this)(this->base + a.first, this->base + b.first);
		}
		
	private:
		bool operator()(const char *a, const char *b)
		{
			return _stricmp(a, b) < 0;
		}

		char *const base;
	};
}

namespace Httpd
{
	HttpRequest::HttpRequest(BufferedReader &reader)
		: reader(reader), remainingLength(0), chunked(false)
		// unnecessary initialization below, just for consistency
		, method(NullOffset), uri(NullOffset), ext(NullOffset), query(NullOffset), host(NullOffset)
		, keepAlive(false), majorVer(0), minorVer(0)
	{}

	void HttpRequest::ParseHeaders()
	{
		bool done = false;
		bool title = true;

		while (!done) {
			char *first = this->reader.ReadLine(true);
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
					throw HttpException(400, true, nullptr);
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
						throw HttpException(400, true, nullptr);
					first += 5;
					this->majorVer = ParseUInt16(first);
					if (*first++ != '.')
						throw HttpException(400, true, nullptr);
					this->minorVer = ParseUInt16(first);
					if (*first != '\0')
						throw HttpException(400, true, nullptr);

					if (this->majorVer != 1)
						throw HttpException(505, true, nullptr);
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
						throw HttpException(400, true, nullptr);
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
				// TODO: Check sensitive win32 names (such as CON, NUL, names with trailing dots and spaces)

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
				this->ext = ext != nullptr ? ext - base : NullOffset;

				title = false;
			} else {
				if (*first == '\0') {
					done = true;
				} else if (*first == ' ' || *first == '\t') {
					// Ignore the standard
					throw HttpException(400, true, nullptr);
				} else {
					char *second = strchr(first, ':');
					if (second == nullptr)
						throw HttpException(400, true, nullptr);
					*second++ = '\0';

					// Eat leading LWS (trailing LWSes are already eaten by BufferedReader::ReadLine)
					while (*second == ' ' || *second == '\t')
						++second;

					// Push headers
					headers.push_back(std::make_pair<UInt16, UInt16>(first - base, second - base));
				}
			}
		}

		char *const base = this->reader.BasePointer();

		// Sort headers for binary searching
		sort(this->headers.begin(), this->headers.end(), HeaderComparePredicate(base));

		// Interpret some headers
		char *hdrHost = this->Header("Host");
		if (this->host == NullOffset && hdrHost != nullptr) {
			char *colon;
			if ((colon = strchr(hdrHost, ':')) != nullptr)
				*colon = '\0';
			this->host = hdrHost - base;
		}

		if (this->majorVer == 1 && this->minorVer >= 1 && this->host == NullOffset)
			throw HttpException(400, false, " (Invalid hostname)");

		char *hdrContentLength = this->Header("Content-Length");
		if (hdrContentLength != nullptr) {
			if ((this->remainingLength = ParseUInt64Dec(hdrContentLength)) == UINT64_MAX)
				throw HttpException(413, true, nullptr);
			this->chunked = false;
		}
		
		char *hdrTransferEncoding = this->Header("Transfer-Encoding");
		if (hdrTransferEncoding != nullptr && _stricmp(hdrTransferEncoding, "identity")) {
			this->remainingLength = 0;
			this->chunked = true;
		}

		char *hdrConnection = this->Header("Connection");
		if (hdrConnection != nullptr) {
			do {
				const char *option = ParseCommaList(hdrConnection);
				if (!_stricmp(option, "close"))
					this->keepAlive = false;
				else if (!_stricmp(option, "Keep-Alive"))
					this->keepAlive = true;
			} while (hdrConnection != nullptr);
		}
	}

	char *HttpRequest::Header(const char *Name)
	{
		char *const base = this->reader.BasePointer();
		auto range = std::equal_range(this->headers.begin(), this->headers.end(), Name,
			HeaderComparePredicate(base));
		
		if (range.first == range.second)
			return nullptr;
		else if (range.first + 1 == range.second)
			return base + range.first->second;
		else
			return nullptr;
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

	void HttpRequest::Flush()
	{
		char buffer[BufferBlockSize];
		while (this->Read(buffer, sizeof(buffer)) != 0);
		this->reader.Flush();
	}

	const char *HttpRequest::Method()
	{
		return reader.BasePointer() + method;
	}

	const char *HttpRequest::URI() {
		return reader.BasePointer() + uri;
	}

	const char *HttpRequest::Extension()
	{
		return ext == NullOffset ? "." : reader.BasePointer() + ext;
	}

	const char *HttpRequest::QueryString()
	{
		return query == NullOffset ? "" : reader.BasePointer() + query;
	}

	const char *HttpRequest::Host()
	{
		return host == NullOffset ? "" : reader.BasePointer() + host;
	}

	HttpVersion HttpRequest::Version()
	{
		return HttpVersion(majorVer, minorVer);
	}

	UInt64 HttpRequest::RemainingLength()
	{
		return remainingLength;
	}

	bool HttpRequest::Chunked()
	{
		return chunked;
	}

	bool HttpRequest::KeepAlive()
	{
		return keepAlive;
	}

	HttpResponse::HttpResponse(Socket &socket, HttpVersion requestVersion, bool requestKeepAlive)
		: socket(socket), socketWriter(socket), writer(socketWriter)
		, assumeKeepAlive(requestVersion.first == 1 && requestVersion.second >= 1)
		, entity(requestVersion.first == 0), keepAlive(requestKeepAlive)
		, chunked(requestVersion.first == 1 && requestVersion.second >= 1)
	{}
	
	void HttpResponse::BeginHeader(const char *status)
	{
		if (this->entity)
			return;

		this->writer.Write("HTTP/1.1 ", 9);
		this->writer.AppendLine(status);
	}

	void HttpResponse::AppendHeader(const char *header)
	{
		if (this->entity)
			return;
		this->writer.AppendLine(header);
	}

	void HttpResponse::AppendHeader(const char *name, const char *value)
	{
		if (this->entity)
			return;
		this->writer.Append(name);
		this->writer.Write(": ", 2);
		this->writer.Append(value);
		this->writer.AppendLine();
	}

	void HttpResponse::AppendHeader(const char *name, UInt64 value)
	{
		if (this->entity)
			return;
		this->writer.Append(name);
		this->writer.Write(": ", 2);
		this->writer.Append(value);
		this->writer.AppendLine();
	}

	void HttpResponse::AppendHeader(const char *name, const char *leading, UInt64 begin, UInt64 end, UInt64 total)
	{
		if (this->entity)
			return;
		this->writer.Append(name);
		this->writer.Write(": ", 2);
		this->writer.Append(leading);
		this->writer.Append(begin);
		this->writer.Write("-", 1);
		this->writer.Append(end);
		this->writer.Write("/", 1);
		this->writer.Append(total);
		this->writer.AppendLine();
	}

	void HttpResponse::EndHeader(bool lengthProvided)
	{
		if (this->entity)
			return;

		// Server header
		this->writer.AppendLine("Server: icyhttpd/0.0");

		// Date header
		char dateBuffer[32];
		GetCurrentDateBuffer(dateBuffer);
		this->writer.Write("Date: ", 6);
		this->writer.AppendLine(dateBuffer);

		if (lengthProvided)
			this->chunked = false;
		else if (!this->chunked)
			this->keepAlive = false;

		// Transfer encoding header
		if (this->chunked)
			this->writer.AppendLine("Transfer-Encoding: chunked");

		// Connection header
		if (!this->keepAlive)
			this->writer.AppendLine("Connection: close");
		else if (!this->assumeKeepAlive)
			this->writer.AppendLine("Connection: keep-alive");

		this->writer.AppendLine();
	}

	void HttpResponse::Flush()
	{
		this->writer.Flush();
	}

	void HttpResponse::Write(const char *buffer, UInt32 size)
	{
		this->writer.Flush();

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
		this->writer.Flush();
		this->socket.TransmitFile(hFile, offset, size);
	}

	void HttpResponse::TransmitFileRange(HANDLE hFile, const pair<UInt64, UInt64> &range)
	{
		this->writer.Flush();
		UInt64 offset = range.first, size = range.second - range.first + 1;

		while (size != 0) {
			UInt32 transmitSize = size >= 0x80000000UL ? 0x80000000UL : static_cast<UInt32>(size);
			this->TransmitFile(hFile, offset, transmitSize);
			offset += transmitSize;
			size -= transmitSize;
		}
	}
}
