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
		HttpException(UInt16 statusCode, bool mustClose)
			: statusCode(statusCode), mustClose(mustClose)
		{}

		UInt16 StatusCode() const { return statusCode; }
		bool MustClose() const { return mustClose; }
	private:
		UInt16 statusCode;
		bool mustClose;
	};

#define DECLARE_EXCEPTION(code, name) \
	class name: public HttpException \
	{ \
	public: \
		name(bool mustClose = false): HttpException(code, mustClose) {} \
	}

	DECLARE_EXCEPTION(400, BadRequestException);
	DECLARE_EXCEPTION(404, NotFoundException);
	DECLARE_EXCEPTION(413, RequestEntityTooLargeException);
	DECLARE_EXCEPTION(501, NotImplementedException);
	DECLARE_EXCEPTION(505, HttpVersionNotSupportedException);
}

#endif
