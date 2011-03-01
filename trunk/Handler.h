#ifndef _HANDLER_H
#define _HANDLER_H

#include "Types.h"
#include "Http.h"

namespace Httpd
{
	class Handler: NonCopyable
	{
	public:
		virtual void Handle(HttpRequest &request, HttpResponse &response) = 0;
		virtual ~Handler() {}
	};
}

#endif
