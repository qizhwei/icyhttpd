#include "Connection.h"
#include "Constant.h"
#include "Dispatcher.h"
#include "Exception.h"
#include "Socket.h"
#include "Stream.h"
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

		printf("[%.3lf] Connection established\n", (double)GetTickCount() / 1000);

		try {
			Reader<Socket> socketReader(conn.socket);
			BufferedReader bufferedReader(socketReader, MaxRequestBufferSize);

			while (true) {
				bool keepAlive = true;
				HttpVersion requestVer(1, 1);
				HttpRequest request(bufferedReader);
				try {
					request.ParseHeaders();
					keepAlive = request.KeepAlive();
					requestVer = request.Version();
					printf("[%.3lf] Request: Host<%s>, URI<%s>\n", (double)GetTickCount() / 1000, request.Host(), request.URI());
					Node &node = ep.GetNode(request.Host());
					// TODO: parse
					Handler &handler = node.GetHandler(request.Extension());
					HttpResponse response(conn.socket, requestVer, keepAlive);
					handler.Handle(node, request, response);
					keepAlive = response.KeepAlive();
				} catch (HttpException &ex) {
					if (ex.MustClose())
						keepAlive = false;
					HttpResponse response(conn.socket, requestVer, keepAlive);
					ex.BuildResponse(response);
					if (ex.MustClose())
						keepAlive = false;
				}
				if (!keepAlive)
					break;
				request.Flush();
			}

			bufferedReader.Flush();
		} catch (const std::exception &) {
			// Other unhandled recoverable exception
			// The connection should be broken now
		}

		printf("[%.3lf] Connection broken\n", (double)GetTickCount() / 1000);
		delete &conn;
	}
}
