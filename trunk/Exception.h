#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include "Types.h"
#include <stdexcept>

namespace Httpd
{
	class HttpResponse;

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
	class SystemException: public std::exception {};
	class NotFoundException: public SystemException {};
	class ForbiddenException: public SystemException {};

	// HTTP exceptions
	class HttpException: public std::exception
	{
	public:
		HttpException(UInt16 statusCode, bool mustClose, const char *addition)
			: statusCode(statusCode), mustClose(mustClose), addition(addition)
		{}

		bool MustClose() const { return mustClose; }
		void BuildResponse(HttpResponse &response) throw();
	private:
		UInt16 statusCode;
		bool mustClose;
		const char *addition;
	};
}

#endif
