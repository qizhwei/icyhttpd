#ifndef _CONNECTION_H
#define _CONNECTION_H

#include "Types.h"

namespace Httpd
{
	class Socket;

	class Connection: NonCopyable
	{
	public:
		Connection(Socket &socket);
		~Connection();

		static void ConnectionCallback(void *param);
	private:
		Socket &socket;
	};
}

#endif