#include "Node.h"

namespace Httpd
{
	Node::Node(const std::string &path, Handler *defaultHandler)
		: path(path), defaultHandler(defaultHandler)
	{
	}
}
