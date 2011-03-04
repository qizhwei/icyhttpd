#include "Connection.h"
#include "Dispatcher.h"
#include "Exception.h"
#include "Socket.h"
#include "Http.h"
#include "Endpoint.h"
#include "Node.h"
#include "Handler.h"
#include <cstdio>

namespace Httpd
{
	Connection::Connection(Endpoint &endpoint, Socket &socket)
		: endpoint(endpoint), socket(socket)
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
		Endpoint &ep = conn.endpoint;

		printf("[Connection] Connection established\n");
		
		try {
			bool keepAlive;
			HttpVersion requestVer(1, 1);
			do {
				try {
					keepAlive = false;
					HttpRequest request(conn.socket);
					keepAlive = request.KeepAlive();
					requestVer = request.Version();

					printf("[Title] Method: %s\n", request.Method());
					printf("[Title] URI: %s\n", request.URI());
					printf("[Title] Extension: %s\n", request.Extension());
					printf("[Title] Query String: %s\n", request.QueryString());
					printf("[Title] Host: %s\n", request.Host());
					printf("[Title] Content-Length: %llu\n", (unsigned long long)request.RemainingLength());
					printf("[Title] Keep-Alive: %s\n", request.KeepAlive() ? "(true)" : "(false)");
					printf("[Title] Chunked: %s\n", request.Chunked() ? "(true)" : "(false)");

					for (size_t i = 0; i != request.HeaderCount(); ++i) {
						HttpHeader header = request.GetHeader(i);
						printf("[Header] %s: %s\n", header.first, header.second);
					}

					Node &node = ep.GetNode(request.Host());
					Handler &handler = node.GetHandler(request.Extension());
					HttpResponse response(conn.socket, requestVer, keepAlive);
					handler.Handle(request, response);

					keepAlive = response.KeepAlive();

					// TODO: Ignore remaining entity data in request

				} catch (const HttpException &ex) {
					HttpResponse response(conn.socket, requestVer, keepAlive);
					response.AppendHeader(HttpHeader("Content-Length", "0"));
					response.EndHeader(ex.StatusCode(), true);
				}
			} while (keepAlive);
		} catch (const std::exception &) {
		}

		printf("[Connection] Connection broken\n");
		delete &conn;
	}
}
