#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include "Types.h"
#include <stdexcept>

namespace Httpd
{
	// The program MUST terminate after fatal exception because the program
	// maybe impossible to continue, some resource leaks may not be handled.
	// Fatal exception SHOULD only be thrown when recovery is not necessary
	// or not possible, such as failure of initializing dispatcher.
	class FatalException {};

	// A std::exception SHOULD be thrown when system error occurs, and the
	// specific type of error has no semantics in this project. For example,
	// when a sharing violation happens when opening a file, this exception
	// is thrown, but the HTTP module don't need to understand this, yet send
	// an Internal Server Error message.

	// Concrete exceptions
	class ResourceInsufficientException: public std::exception {};
	class SystemException: public std::exception {};

	// HTTP exceptions
	class HttpException: public std::exception
	{
	public:
		HttpException(UInt16 statusCode, bool mustClose, const char *reason, const char *html)
			: statusCode(statusCode), mustClose(mustClose), reason(reason), html(html)
		{}

		UInt16 StatusCode() const { return statusCode; }
		bool MustClose() const { return mustClose; }
		const char *Reason() const { return reason; }
		const char *Html() const { return html; }
	private:
		UInt16 statusCode;
		bool mustClose;
		const char *reason;
		const char *html;
	};

#define DECLARE_EXCEPTION(code, name, reason, html) \
	class name: public HttpException \
	{ \
	public: \
		name(bool mustClose = false): HttpException(code, mustClose, reason, html) {} \
	}

	DECLARE_EXCEPTION(400, BadRequestException, "Bad Request", "<h1>Bad Request</h1>");
	DECLARE_EXCEPTION(403, ForbiddenException, "Forbidden", "<h1>Forbidden</h1>");
	DECLARE_EXCEPTION(404, NotFoundException, "Not Found", "<h1>Not Found</h1>");

	// TODO: An Allow header field MUST be present in a 405 (Method Not Allowed) response.
	// virtual void HttpException::BuildHttpResponse(HttpResponse &);
	DECLARE_EXCEPTION(405, MethodNotAllowedException, "Method Not Allowed", "<h1>Method Not Allowed</h1>");
	DECLARE_EXCEPTION(413, RequestEntityTooLargeException, "Request Entity Too Large", "<h1>Request Entity Too Large</h1>");
	DECLARE_EXCEPTION(501, NotImplementedException, "Not Implemented", "<h1>Not Implemented</h1>");
	DECLARE_EXCEPTION(505, HttpVersionNotSupportedException, "HTTP Version Not Supported", "<h1>HTTP Version Not Supported</h1>");
}

#endif
