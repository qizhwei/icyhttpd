#include "Endpoint.h"
#include "Types.h"
#include "Dispatcher.h"
#include "Connection.h"
#include "Constant.h"
#include <unordered_map>
#include <string>

namespace Httpd
{
	Endpoint::Endpoint(const std::string &ip, UInt16 port, Node *defaultNode)
		: defaultNode(defaultNode)
	{
		socket.BindIP(ip.c_str(), port);
		socket.Listen();

		for (int i = 0; i < AcceptFiberCount; ++i)
			Dispatcher::Instance().Queue(&AcceptCallback, this);
	}

	void Endpoint::AcceptCallback(void *param)
	{
		Endpoint &ep = *static_cast<Endpoint *>(param);

		while (true) {
			try {
				Socket *socket = new Socket();
				try {
					ep.socket.Accept(*socket);
					new Connection(*socket);
				} catch (...) {
					delete socket;
					throw;
				}
			} catch (const std::exception &) {
			}
		}
	}
}
