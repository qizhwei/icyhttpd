#ifndef _NODE_H
#define _NODE_H

#include "Types.h"
#include "Utility.h"
#include <string>
#include <unordered_map>

namespace Httpd
{
	class Handler;

	class Node: NonCopyable
	{
	public:
		Node(const std::string &path, Handler *defaultHandler);
		Handler &GetHandler(const CiString &ext);

	private:
		// this class has no destructor, declaration only
		~Node();
	private:
		std::string path;
		std::unordered_map<CiString, Handler *> bindings;
		Handler *defaultHandler;
	};
}

#endif
