#ifndef _HANDLER_H
#define _HANDLER_H

#include "Types.h"
#include "Http.h"
#include "Node.h"

namespace Httpd
{
	class Handler: NonCopyable
	{
	public:
		virtual void Handle(Node &node, HttpRequest &request, HttpResponse &response) = 0;
	};
}

#endif
