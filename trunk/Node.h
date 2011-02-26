#ifndef _NODE_H
#define _NODE_H

#include "Types.h"
#include <string>

namespace Httpd
{
	class Handler;

	class Node: NonCopyable
	{
	public:
		Node(const std::string &path, Handler *defaultHandler);

	private:
		// this class has no destructor, declaration only
		~Node();
	private:
		std::string path;
		Handler *defaultHandler;
	};
}

#endif
