#include "Http.h"
#include "Types.h"
#include "Constant.h"
#include "Exception.h"
#include "Win32.h"
#include <cstring>
#include <utility>
#include <unordered_map>

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
}

namespace Httpd
{
	HttpRequest::HttpRequest(Readable &stream)
		: stream(stream), buffer(MinRequestBufferSize * 2), begin(0), end(0), contentLength(NullOffset), chunked(false)
	{
		bool done = false;
		bool title = true;

		while (!done) {
			UInt32 bytesRead = stream.Read(&this->buffer[this->end], this->buffer.size() - this->end);

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
						if ((first = strchr(first, ' ')) == nullptr) {
							this->majorVer = 0;
							this->minorVer = 9;
							this->keepAlive = false;

							// HTTP/0.9 requests don't have headers
							done = true;
						} else {
							*first++ = '\0';
							// TODO: parse HTTP/xx.xx from `first'
							this->majorVer = 1;
							this->minorVer = 1;

							if (this->majorVer != 1)
								throw HttpVersionNotSupportedException();
							if (this->minorVer == 0)
								this->keepAlive = false;
							else
								this->keepAlive = true;
						}

						// Request URI
						if (*uri == '/') {
							++uri;
							this->host = NullOffset;
						} else {
							if (_strnicmp(uri, "http://", 7))
								throw BadRequestException();
							uri += 7;
							this->host = uri - base;
							if ((uri = strchr(uri, '/')) == nullptr)
								throw BadRequestException();
							*uri++ = '\0';
						}

						// TODO: URI Decode and Rewrite
						this->uri = uri - base;

						// Query string
						char *query;
						if ((query = strchr(uri, '?')) != nullptr) {
							*query++ = '\0';
							this->query = query - base;
						} else {
							this->query = NullOffset;
						}

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
								// TODO: parse Int64
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

	HttpResponse::HttpResponse(Writable &stream, HttpVersion requestVer)
		: stream(stream), buffer(TitleSize), requestVer(requestVer), titleOffset(TitleSize), entity(false), chunked(false)
	{}

	void HttpResponse::AppendTitle(UInt16 status)
	{
		if (this->requestVer.first == 0 || this->entity)
			return;

		wsprintfA(&*this->buffer.begin(), "HTTP/1.1 %u %s\r\n",
			static_cast<unsigned int>(status), HttpUtility::Instance().ReasonPhrase(status));
		const size_t titleSize = strlen(&*this->buffer.begin());
		this->titleOffset = TitleSize - titleSize;
		memmove(&this->buffer[this->titleOffset], &*this->buffer.begin(), titleSize);
	}

	void HttpResponse::AppendHeader(HttpHeader header)
	{
		if (this->requestVer.first == 0)
			return;

		if (!this->entity) {
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
		} else {
			// TODO: Chunked encoding supports header in entity
		}
	}

	void HttpResponse::EndHeader()
	{
		if (this->requestVer.first == 0 || this->entity)
			return;

		const char *crlf = "\r\n";
		this->buffer.insert(this->buffer.end(), crlf, crlf + 2);
		this->Flush();
		this->entity = true;
	}

	void HttpResponse::Write(const char *buffer, UInt32 size)
	{
		if (!this->entity)
			this->EndHeader();

		stream.Write(buffer, size);
	}

	void HttpResponse::Flush()
	{
		const size_t size = this->buffer.size() - this->titleOffset;
		if (size != 0) {
			stream.Write(&this->buffer[this->titleOffset], size);
			vector<char>().swap(this->buffer);
			this->titleOffset = 0;
		}
	}
}

namespace
{
	std::unordered_map<UInt16, const char *> reason;

	void setReason(UInt16 status, const char *phrase)
	{
		reason.insert(make_pair(status, phrase));
	}
}

namespace Httpd
{
	HttpUtility &HttpUtility::Instance()
	{
		HttpUtility *u(new HttpUtility());
		return *u;
	}

	HttpUtility::HttpUtility()
	{
		setReason(100, "Continue");
		setReason(101, "Switching Protocols");
		setReason(200, "OK");
		setReason(201, "Created");
		setReason(202, "Accepted");
		setReason(203, "Non-Authoritative Information");
		setReason(204, "No Content");
		setReason(205, "Reset Content");
		setReason(206, "Partial Content");
		setReason(300, "Multiple Choices");
		setReason(301, "Moved Permanently");
		setReason(302, "Found");
		setReason(303, "See Other");
		setReason(304, "Not Modified");
		setReason(305, "Use Proxy");
		// 306 is unused and reserved
		setReason(307, "Temporary Redirect");
		setReason(400, "Bad Request");
		setReason(401, "Unauthorized");
		setReason(402, "Payment Required");
		setReason(403, "Forbidden");
		setReason(404, "Not Found");
		setReason(405, "Method Not Allowed");
		setReason(406, "Not Acceptable");
		setReason(407, "Proxy Authentication Required");
		setReason(408, "Request Timeout");
		setReason(409, "Conflict");
		setReason(410, "Gone");
		setReason(411, "Length Required");
		setReason(412, "Precondition Failed");
		setReason(413, "Request Entity Too Large");
		setReason(414, "Request-URI Too Long");
		setReason(415, "Unsupported Media Type");
		setReason(416, "Request Range Not Satisfiable");
		setReason(417, "Expectation Failed");
		setReason(500, "Internal Server Error");
		setReason(501, "Not Implemented");
		setReason(502, "Bad Gateway");
		setReason(503, "Service Unavailable");
		setReason(504, "Gateway Timeout");
		setReason(505, "HTTP Version Not Supported");
	}

	const char *HttpUtility::ReasonPhrase(UInt16 status)
	{
		auto i = reason.find(status);
		if (i == reason.end())
			throw SystemException();
		return i->second;
	}
}
