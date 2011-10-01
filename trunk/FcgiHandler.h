#ifndef _FASTCGI_H
#define _FASTCGI_H

#include "Types.h"
#include "Utility.h"
#include "Handler.h"
#include "Dispatcher.h"

namespace Httpd
{
	class FcgiHandler: public Handler
	{
	public:
		virtual void Handle(Node &node, HttpRequest &request, HttpResponse &response);
	};
}

#endif
