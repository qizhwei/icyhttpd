#include "Connection.h"
#include "Dispatcher.h"
#include "Socket.h"
#include "Request.h"
#include "Response.h"
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
		
		try {
			while (true) {
				Request request(conn.socket);

				printf("Method: %s\n", request.Method());
				printf("URI: %s\n", request.URI());
				printf("Query String: %s\n", request.QueryString());
				printf("Host: %s\n", request.Host());
				printf("Content-Length: %lld\n", request.ContentLength());
				printf("Keep-Alive: %d\n", request.KeepAlive());
				printf("Chunked: %d\n", request.Chunked());

				for (size_t i = 0; i != request.HeaderCount(); ++i) {
					Request::Header header = request.GetHeader(i);
					printf("[Header] %s: %s\n", header.first, header.second);
				}

				throw NotImplementedException();
			}
		} catch (const HttpException &ex) {
			try {
				Response response(conn.socket, ex.StatusCode());
			} catch (const Exception &) {
			}
		} catch (const Exception &) {
		}

		delete &conn;
	}
}
