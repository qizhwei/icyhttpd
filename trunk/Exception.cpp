#include "Exception.h"
#include "Types.h"

namespace Httpd
{
	Int16 NotFoundException::StatusCode() const
	{
		return 404;
	}

	Int16 NotImplementedException::StatusCode() const
	{
		return 501;
	}

	Int16 BadRequestException::StatusCode() const
	{
		return 400;
	}

	Int16 HttpVersionNotSupportedException::StatusCode() const
	{
		return 505;
	}
}
