#include "Endpoint.h"
#include "Types.h"
#include "Dispatcher.h"
#include "Connection.h"
#include "Constant.h"
#include "Node.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <utility>

using namespace Httpd;
using namespace std;

namespace Httpd
{
	Endpoint::Endpoint(const std::string &ip, UInt16 port, Node *defaultNode)
		: defaultNode(defaultNode)
	{
		socket.CreateListenerIpv4(ip.c_str(), port);

		for (int i = 0; i < AcceptFiberCount; ++i) {
			Dispatcher::Instance().Queue([this](){
				while (true) {
					try {
						unique_ptr<Socket> socket(new Socket());
						socket->CreateClientIpv4();
						this->socket.Accept(*socket);
						Connection::Create(*this, move(socket));
					} catch (const std::exception &) {
					}
				}
			});
		}
	}

	Node &Endpoint::GetNode(const CiString &host)
	{
		auto iter = this->bindings.find(host);
		if (iter == this->bindings.end())
			return *(this->defaultNode);
		else
			return *(iter->second);
	}
}
