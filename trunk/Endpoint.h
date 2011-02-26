#ifndef _SERVER_H
#define _SERVER_H

#include "Types.h"
#include "Socket.h"
#include <unordered_map>
#include <string>

namespace Httpd
{
	class Node;

	class Endpoint: NonCopyable
	{
	public:
		Endpoint(const std::string &ip, UInt16 port, Node *defaultNode);

	private:
		// this class has no destructor, declaration only
		~Endpoint();

		static void AcceptCallback(void *param);
	private:
		Socket socket;
		std::unordered_map<std::string, Node *> bindings;
		Node *defaultNode;
	};
}

#endif
