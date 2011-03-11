#include "Node.h"
#include "Utility.h"

namespace Httpd
{
	Node::Node(const std::string &path, Handler *defaultHandler)
		: pathA(path), pathW(MB2WC(CP_ACP, path)), defaultHandler(defaultHandler)
	{}

	Handler &Node::GetHandler(const CiString &ext)
	{
		auto iter = this->bindings.find(ext);
		if (iter == this->bindings.end())
			return *(this->defaultHandler);
		else
			return *(iter->second);
	}
}
