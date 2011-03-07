#ifndef _HANDLER_H
#define _HANDLER_H

#include "Types.h"

namespace Httpd
{
	class Node;
	class HttpRequest;
	class HttpResponse;

	class Handler: NonCopyable
	{
	public:
		virtual void Handle(Node &node, HttpRequest &request, HttpResponse &response) = 0;
	};
}

#endif
