#ifndef _ECHOHANDLER_H
#define _ECHOHANDLER_H

#include "Handler.h"
#include "Http.h"

namespace Httpd
{
	class EchoHandler: public Handler
	{
		virtual void Handle(HttpRequest &request, HttpResponse &response);
	};
}

#endif
