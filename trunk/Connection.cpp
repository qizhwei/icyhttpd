#include "Connection.h"
#include "Dispatcher.h"
#include "Socket.h"
#include <cstdio>

namespace Httpd
{
	Connection::Connection(Socket &socket)
		: socket(socket)
	{
		Dispatcher::Instance().Queue(&ConnectionCallback, this);
	}
	
	Connection::~Connection()
	{
		delete &socket;
	}

	void Connection::ConnectionCallback(void *param)
	{
		Connection &conn = *static_cast<Connection *>(param);
		printf("Connection!\n");
		delete &conn;
	}
}
