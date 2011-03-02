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
			if ((i = i * 10 + (*p - '0')) > 65535)
				i = 65535;
			++p;
		}
		return i;
	}

	char *Weekdays[7] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
	};

	char *Months[12] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	};
}

namespace Httpd
{
	HttpRequest::HttpRequest(Socket &socket)
		: socket(socket), buffer(MinRequestBufferSize * 2), begin(0), end(0), contentLength(0), chunked(false)
	{
		bool done = false;
		bool title = true;

		while (!done) {
			UInt32 bytesRead = socket.Read(&this->buffer[this->end], this->buffer.size() - this->end);

			if (bytesRead == 0)
				throw SystemException();

			for (size_t i = this->end; !done && i != this->end + bytesRead; ++i) {
				if (this->buffer[i] == '\n') {
					char *const base = &*this->buffer.begin();
					char *first = &this->buffer[this->begin];
					char *last = &this->buffer[i];

					// Eat CR and LF
					*last = '\0';
					if (first != last && last[-1] == '\r')
						*--last = '\0';

					// Empty lines are ignored
					if (title && first != last) {

						// Request method
						this->method = first - base;
						if ((first = strchr(first, ' ')) == nullptr)
							throw BadRequestException();
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
								throw BadRequestException();
							first += 5;
							this->majorVer = ParseUInt16(first);
							if (*first++ != '.')
								throw BadRequestException();
							this->minorVer = ParseUInt16(first);
							if (*first != '\0')
								throw BadRequestException();

							if (this->majorVer != 1)
								throw HttpVersionNotSupportedException();
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
								throw BadRequestException();
							char *host = uri;
							if ((uri = strchr(uri + 7, '/')) == nullptr)
								uri = uriLast;
							memmove(host, host + 7, uri - host - 7);
							host[uri - host - 7] = '\0';
							if (uri == uriLast)
								*--uri = '/';
							uri[-1] = '\0';

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
					} else if (!title) {
						if (*first == '\0') {
							done = true;
						} else if (*first == ' ' || *first == '\t') {
							// Ignore the standard
							throw BadRequestException();
						} else {
							char *second = strchr(first, ':');
							if (second == nullptr)
								throw BadRequestException();
							*second++ = '\0';

							// Eat leading and trailing LWS
							while (*second == ' ' || *second == '\t')
								++second;
							while (last[-1] == ' ' || last[-1] == '\t')
								*--last = '\0';

							this->headers.push_back(std::pair<Int16, Int16>(first - base, second - base));

							// Filter out known headers
							if (this->host == NullOffset && !_stricmp(first, "Host")) {
								this->host = second - base;
							} else if (!_stricmp(first, "Content-Length")) {
								// TODO: parse UInt64
							} else if (!_stricmp(first, "Transfer-Encoding")) {
								if (_stricmp(second, "identity"))
									this->chunked = true;
							} else if (!_stricmp(first, "Connection")) {
								do {
									const char *option = ParseCommaList(second);
									if (!_stricmp(option, "close"))
										this->keepAlive = false;
									else if (!_stricmp(option, "Keep-Alive"))
										this->keepAlive = true;
								} while (second != nullptr);
							}
						}
					}

					this->begin = i + 1;
				}
			}
			this->end += bytesRead;
			if (!done && this->buffer.size() - this->end < MinRequestBufferSize) {
				size_t newSize = this->end + MinRequestBufferSize * 2;
				if (newSize >= MaxRequestBufferSize)
					throw BadRequestException();
				this->buffer.resize(newSize);
			}
		}

		// TODO: Throw BadRequestException if majorVer = 1, minorVer >= 1 and host not presented
	}

	UInt32 HttpRequest::Read(char *buffer, UInt32 size)
	{
		// TODO: first read from [this->begin, this->end), then read through
		throw NotImplementedException();
	}

	HttpHeader HttpRequest::GetHeader(size_t index)
	{
		std::pair<Int16, Int16> offsets = this->headers[index];
		const char *first = &this->buffer[offsets.first];
		const char *second = &this->buffer[offsets.second];
		return HttpHeader(first, second);
	}

	void HttpRequest::Flush()
	{
		// TODO: Not implemented
		throw NotImplementedException();
	}

	HttpResponse::HttpResponse(Socket &socket, HttpVersion requestVersion, bool requestKeepAlive)
		: assumeKeepAlive(requestVersion.first == 1 && requestVersion.second >= 1)
		, entity(requestVersion.first == 0), keepAlive(requestKeepAlive)
		, chunked(requestVersion.first == 1 && requestVersion.second >= 1)
		, socket(socket), buffer(TitleSize), titleOffset(TitleSize)
	{}

	void HttpResponse::AppendTitle(UInt16 status)
	{
		if (this->entity)
			return;

		wsprintfA(&*this->buffer.begin(), "HTTP/1.1 %u %s\r\n",
			static_cast<unsigned int>(status), HttpUtility::Instance().ReasonPhrase(status));
		const size_t titleSize = strlen(&*this->buffer.begin());
		this->titleOffset = TitleSize - titleSize;
		memmove(&this->buffer[this->titleOffset], &*this->buffer.begin(), titleSize);
	}

	void HttpResponse::AppendHeader(HttpHeader header)
	{
		if (this->entity)
			return;

		const size_t first_len = strlen(header.first);
		const char *colon = ": ";
		const size_t second_len = strlen(header.second);
		const char *crlf = "\r\n";
		const size_t newSize = this->buffer.size() + first_len + second_len + 4;

		if (newSize >= MaxResponseHeaderSize)
			throw SystemException();

		this->buffer.reserve(newSize);
		this->buffer.insert(this->buffer.end(), header.first, header.first + first_len);
		this->buffer.insert(this->buffer.end(), colon, colon + 2);
		this->buffer.insert(this->buffer.end(), header.second, header.second + second_len);
		this->buffer.insert(this->buffer.end(), crlf, crlf + 2);
	}

	void HttpResponse::EndHeader(bool lengthProvided)
	{
		if (this->entity)
			return;

		if (lengthProvided)
			this->chunked = false;
		else if (!this->chunked)
			this->keepAlive = false;

		if (this->chunked)
			AppendHeader(HttpHeader("Transfer-Encoding", "chunked"));

		if (!this->keepAlive)
			AppendHeader(HttpHeader("Connection", "close"));
		else if (!this->assumeKeepAlive)
			AppendHeader(HttpHeader("Connection", "keep-alive"));

		AppendHeader(HttpHeader("Server", "icyhttpd/0.0"));

		__time64_t currentTime;
		tm currentTm;

		char dateBuffer[32];
		_time64(&currentTime);
		_gmtime64_s(&currentTm, &currentTime);
        wsprintfA(dateBuffer, "%3s, %02d %3s %04d %02d:%02d:%02d GMT",
            Weekdays[currentTm.tm_wday], currentTm.tm_mday, Months[currentTm.tm_mon],
			currentTm.tm_year + 1900, currentTm.tm_hour, currentTm.tm_min, currentTm.tm_sec);
		AppendHeader(HttpHeader("Date", dateBuffer));

		const char *crlf = "\r\n";
		this->buffer.insert(this->buffer.end(), crlf, crlf + 2);
		this->Flush();
		this->entity = true;
	}

	void HttpResponse::Write(const char *buffer, UInt32 size)
	{
		assert(this->entity);
		socket.Write(buffer, size);
	}

	void HttpResponse::Flush()
	{
		const size_t size = this->buffer.size() - this->titleOffset;
		if (size != 0) {
			socket.Write(&this->buffer[this->titleOffset], size);
			vector<char>().swap(this->buffer);
			this->titleOffset = 0;
		}
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
