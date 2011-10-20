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
		Node(std::string path, Handler *defaultHandler);
		bool AddHandler(const CiString &ext, Handler *handler);
		Handler &GetHandler(const CiString &ext);
		const std::string &PathA() { return pathA; }
		const std::wstring &PathW() { return pathW; }

	private:
		// this class has no destructor, declaration only
		~Node();
	private:
		std::string pathA;
		std::wstring pathW;
		std::unordered_map<CiString, Handler *> bindings;
		Handler *defaultHandler;
	};
}

#endif
