#include "Node.h"
#include "Utility.h"
#include <utility>

namespace Httpd
{
	Node::Node(std::string path, Handler *defaultHandler)
		: pathA(move(path)), pathW(MB2WC(CP_ACP, pathA)), defaultHandler(defaultHandler)
	{}

	Handler &Node::GetHandler(const CiString &ext)
	{
		auto iter = this->bindings.find(ext);
		if (iter == this->bindings.end())
			return *(this->defaultHandler);
		else
			return *(iter->second);
	}

	bool Node::AddHandler(const CiString &ext, Handler *handler)
	{
		return this->bindings.insert(make_pair(ext, handler)).second;
	}
}
