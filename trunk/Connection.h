#ifndef _CONNECTION_H
#define _CONNECTION_H

#include "Types.h"

namespace Httpd
{
	class Endpoint;
	class Socket;

	class Connection: NonCopyable
	{
	public:
		Connection(Endpoint &endpoint, Socket &socket);
		~Connection();

		static void ConnectionCallback(void *param);
	private:
		Endpoint &endpoint;
		Socket &socket;
	};
}

#endif
