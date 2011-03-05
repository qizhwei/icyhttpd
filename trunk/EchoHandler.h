#ifndef _ECHOHANDLER_H
#define _ECHOHANDLER_H

#include "Handler.h"

namespace Httpd
{
	class EchoHandler: public Handler
	{
	public:
		virtual void Handle(Node &node, HttpRequest &request, HttpResponse &response);
	private:
		~EchoHandler();
	};
}

#endif
