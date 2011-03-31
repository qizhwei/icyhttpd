#ifndef _CONNECTION_H
#define _CONNECTION_H

#include "Types.h"
#include <memory>

namespace Httpd
{
	class Endpoint;
	class Socket;

	class Connection: NonCopyable
	{
	public:
		Connection(Endpoint &endpoint, std::unique_ptr<Socket> socket);

		static void ConnectionCallback(void *param);
	private:
		Endpoint &endpoint;
		std::unique_ptr<Socket> socket;
	};
}

#endif
