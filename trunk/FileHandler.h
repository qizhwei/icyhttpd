#ifndef _FILEHANDLER_H
#define _FILEHANDLER_H

#include "Handler.h"

namespace Httpd
{
	class FileHandler: public Handler
	{
	public:
		virtual void Handle(Node &node, HttpRequest &request, HttpResponse &response);
	private:
		~FileHandler();
	};
}

#endif
