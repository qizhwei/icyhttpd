#ifndef _LOADER_H
#define _LOADER_H

#include "Utility.h"
#include "Handler.h"
#include "Endpoint.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include <memory>
#include <map>
#include <vector>
#include <utility>
#include <string>

namespace Httpd
{
	struct SiteData
	{
		const char *root;
		std::vector<std::pair<const char *, int> > listens;
		std::vector<const char *> hosts;
		std::vector<std::pair<const char *, const char *> > handlers;
	};

	class Loader: NonCopyable
	{
	public:
		Loader(const char *filename);
		void LoadConfig();
	private:
		rapidxml::file<> file;
		rapidxml::xml_document<> doc;
		std::vector<SiteData> sites;
		std::map<CiString, Handler *> handlerNs;
		std::map<std::pair<CiString, int>, Endpoint *> eps;
	private:
		void ParseRoot(rapidxml::xml_node<> *xn);
		void ParseSite(rapidxml::xml_node<> *xn);
		void ParseFastcgi(rapidxml::xml_node<> *xn);
	};
}

#endif
