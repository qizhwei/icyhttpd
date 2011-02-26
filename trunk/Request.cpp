#include "Request.h"
#include "Constant.h"
#include "Exception.h"
#include <cstring>
#include <utility>

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
	Request::Request(Readable &stream)
		: stream(stream), buffer(MinRequestBufferSize * 2), begin(0), end(0), contentLength(-1), chunked(false)
	{
		bool done = false;
		bool title = true;

		while (!done) {
			UInt32 bytesRead = stream.Read(&this->buffer[this->end], this->buffer.size() - this->end);
			for (size_t i = this->end; !done && i != this->end + bytesRead; ++i) {
				if (this->buffer[i] == '\n') {
					char *const base = &*this->buffer.begin();
					char *first = &this->buffer[this->begin];
					char *last = &this->buffer[i];

					// Eat CR and LF
					*last = '\0';
					if (first != last && last[-1] == '\r')
						*--last = '\0';

					if (title) {
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
							this->host = -1;
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
							this->query = -1;
						}

						title = false;
					} else {
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
							if (this->host == -1 && !_stricmp(first, "Host")) {
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

	UInt32 Request::Read(char *buffer, UInt32 size)
	{
		// TODO: first read from [this->begin, this->end), then read through
		throw NotImplementedException();
	}

	Request::Header Request::GetHeader(size_t index)
	{
		std::pair<Int16, Int16> offsets = this->headers[index];
		const char *first = &this->buffer[offsets.first];
		const char *second = &this->buffer[offsets.second];
		return Request::Header(first, second);
	}
}
