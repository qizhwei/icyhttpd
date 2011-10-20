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
#include "Exception.h"

using namespace Httpd;
using namespace std;

namespace Httpd
{
	Endpoint::Endpoint(const std::string &ip, UInt16 port)
		: defaultNode(nullptr)
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
		if (iter == this->bindings.end()) {
			if (this->defaultNode) {
				return *(this->defaultNode);
			} else {
				throw HttpException(400, false, " (Invalid Hostname)");
			}
		} else {
			return *(iter->second);
		}
	}

	bool Endpoint::SetDefaultNode(Node *defaultNode)
	{
		if (!this->defaultNode) {
			this->defaultNode = defaultNode;
			return true;
		} else {
			return false;
		}
	}

	bool Endpoint::AddBinding(const CiString &host, Node *node)
	{
		return this->bindings.insert(make_pair(host, node)).second;
	}
}
