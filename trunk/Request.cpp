#include "Request.h"
#include "Constant.h"
#include "Exception.h"
#include <cstring>
#include <utility>

using namespace std;

namespace Httpd
{
	Request::Request(Readable &stream)
		: stream(stream), buffer(MinRequestBufferSize * 2), begin(0), end(0)
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
						}

						// Request URI
						if (*uri == '/') {
							this->uri = uri + 1 - base;
							this->host = -1;
						} else if (!strnicmp(uri, "http://", 7)) {
							uri += 7;
							this->host = uri - base;
							if ((uri = strchr(uri, '/')) == nullptr)
								throw BadRequestException();
							*uri++ = '\0';
							this->uri = uri - base;
						} else {
							throw BadRequestException();
						}

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
								*second++ = '\0';
							while (last[-1] == ' ' || last[-1] == '\t')
								*--last = '\0';

							// TODO: Filter out known headers (such as Host, Content-Length, Transfer-Encoding, etc.)
							this->headers.push_back(std::pair<Int16, Int16>(first - base, second - base));
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

	Request::~Request()
	{
	}

	UInt32 Request::Read(char *buffer, UInt32 size)
	{
		// TODO: first read from [this->begin, this->end), then read through
		throw NotImplementedException();
	}
}
