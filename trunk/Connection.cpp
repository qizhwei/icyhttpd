#include "Connection.h"
#include "Dispatcher.h"
#include "Socket.h"
#include "Request.h"
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
		
		while (true) {
			Request request(conn.socket);

			printf("Method: %s\n", request.Method());
			printf("URI: %s\n", request.URI());
			printf("Query String: %s\n", request.QueryString());
			printf("Host: %s\n", request.Host());

			break;
		}

		delete &conn;
	}
}
