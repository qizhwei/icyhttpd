#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include "Types.h"

namespace Httpd
{
	// The program MUST terminate after fatal exception because the program
	// maybe impossible to continue, some resource leaks may not be handled.
	// Fatal exception SHOULD only be thrown when recovery is not necessary
	// or not possible, such as failure of initializing dispatcher.
	class FatalException {};

	// A general exception SHOULD be thrown when system error occurs, and the
	// specific type of error has no semantics in this project. For example,
	// when a sharing violation happens when opening a file, this exception
	// is thrown, but the HTTP module don't need to understand this, yet send
	// an Internal Server Error message.
	class Exception
	{
	public:
		virtual ~Exception() {}
	};

	// HTTP Exceptions
	class HttpException: public Exception
	{
	public:
		virtual Int16 StatusCode() const = 0;
	};

	class NotFoundException: public HttpException
	{
	public:
		virtual Int16 StatusCode() const;
	};

	class NotImplementedException: public HttpException
	{
	public:
		virtual Int16 StatusCode() const;
	};

	class BadRequestException: public HttpException
	{
	public:
		virtual Int16 StatusCode() const;
	};

	class HttpVersionNotSupportedException: public HttpException
	{
	public:
		virtual Int16 StatusCode() const;
	};
}

#endif
