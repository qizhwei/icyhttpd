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

		printf("connection established\n");
		
		try {
			bool keepAlive;
			HttpVersion requestVer(1, 1);
			do {
				try {
					keepAlive = false;
					HttpRequest request(conn.socket);
					keepAlive = request.KeepAlive();
					requestVer = request.Version();

					printf("Method: %s\n", request.Method());
					printf("URI: %s\n", request.URI());
					printf("Extension: %s\n", request.Extension());
					printf("Query String: %s\n", request.QueryString());
					printf("Host: %s\n", request.Host());
					printf("Content-Length: %lld\n", request.ContentLength());
					printf("Keep-Alive: %s\n", request.KeepAlive() ? "(true)" : "(false)");
					printf("Chunked: %s\n", request.Chunked() ? "(true)" : "(false)");

					for (size_t i = 0; i != request.HeaderCount(); ++i) {
						HttpHeader header = request.GetHeader(i);
						printf("[Header] %s: %s\n", header.first, header.second);
					}

					Node &node = ep.GetNode(request.Host());
					Handler &handler = node.GetHandler(request.Extension());
					HttpResponse response(conn.socket, requestVer);
					handler.Handle(request, response);

					// TODO: Implement
					throw NotImplementedException();

				} catch (const HttpException &ex) {
					HttpResponse response(conn.socket, requestVer);
					response.AppendTitle(ex.StatusCode());
					response.AppendHeader(HttpHeader("Connection", keepAlive ? "keep-alive" : "close"));
					response.AppendHeader(HttpHeader("Content-Length", "0"));
					response.EndHeader();
					response.Flush();
				}
			} while (keepAlive);
		} catch (const std::exception &) {
		}

		printf("connection broken\n");
		delete &conn;
	}
}
