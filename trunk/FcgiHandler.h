#ifndef _FCGIHANDLER_H
#define _FCGIHANDLER_H

#include "Handler.h"

namespace Httpd
{
	class FcgiHandler: public Handler
	{
	public:
		virtual void Handle(Node &node, HttpRequest &request, HttpResponse &response);
	};
}

#endif
