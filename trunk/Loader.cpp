#include "Loader.h"
#include "FcgiHandler.h"
#include "rapidxml.hpp"
#include "rapidxml_iterators.hpp"
#include "rapidxml_utils.hpp"
#include <stdexcept>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <utility>

using namespace Httpd;
using namespace std;
using namespace rapidxml;

namespace
{
	class LoaderException: public exception
	{
	};

	void Warning(const char *format, ...)
	{
		va_list args;
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
	}

	void Error(const char *format, ...)
	{
		va_list args;
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
		throw LoaderException();
	}
}

namespace Httpd
{
	Loader::Loader(const char *filename)
		: file(filename)
	{
	}

	void Loader::LoadConfig()
	{
		try {
			doc.parse<parse_no_data_nodes | parse_trim_whitespace>(file.data());
			xml_node<> *root = doc.first_node("icyhttpd");
			if (root == nullptr) {
				Error("root node \'icyhttpd\' not exists\n");
			}
			ParseRoot(root);

			// TODO: Build sites

		} catch (const LoaderException &) {
		}
	}

	void Loader::ParseRoot(rapidxml::xml_node<> *xn)
	{
		for (node_iterator<char> i(xn); i != node_iterator<char>(); ++i) {
			const char *name = i->name();
			if (!_stricmp(name, "site")) {
				ParseSite(&*i);
			} else if (!_stricmp(name, "fastcgi")) {
				ParseFastcgi(&*i);
			} else {
				Warning("undefined node \'%s\'\n", name);
			}
		}
	}

	void Loader::ParseSite(rapidxml::xml_node<> *xn)
	{
		SiteData sd;

		for (node_iterator<char> i(xn); i != node_iterator<char>(); ++i) {
			const char *name = i->name();
			if (!_stricmp(name, "listen")) {
				xml_attribute<> *ip = i->first_attribute("ip");
				xml_attribute<> *port = i->first_attribute("port");
				sd.listens.push_back(make_pair(
					ip ? ip->value() : "",
					port ? atoi(port->value()) : 0));
			} else if (!_stricmp(name, "host")) {
				xml_attribute<> *name = i->first_attribute("name");
				sd.hosts.push_back(name ? name->value() : "");
			} else if (!_stricmp(name, "mount")) {
				xml_attribute<> *url = i->first_attribute("url");
				xml_attribute<> *path = i->first_attribute("path");
				if (!url) {
					Warning("node \'mount\' must have attribute \'url\'\n");
					continue;
				}
				if (!path) {
					Warning("node \'mount\' must have attribute \'path\'\n");
					continue;
				}
				sd.mounts.push_back(make_pair(url->value(), path->value()));
			} else if (!_stricmp(name, "handler")) {
				xml_attribute<> *extension = i->first_attribute("extension");
				xml_attribute<> *ref = i->first_attribute("ref");
				if (!extension) {
					Warning("node \'mount\' must have attribute \'extension\'\n");
					continue;
				}
				if (!ref) {
					Warning("node \'mount\' must have attribute \'ref\'\n");
					continue;
				}
				sd.handlers.push_back(make_pair(extension->value(), ref->value()));
			} else {
				Warning("undefined node \'%s\'\n", name);
			}
		}

		sites.push_back(move(sd));
	}

	void Loader::ParseFastcgi(rapidxml::xml_node<> *xn)
	{
		xml_attribute<> *id = xn->first_attribute("id");
		xml_attribute<> *path = xn->first_attribute("path");
		xml_attribute<> *queueLength = xn->first_attribute("queueLength");
		xml_attribute<> *maxInstances = xn->first_attribute("maxInstances");
		xml_attribute<> *idleTime = xn->first_attribute("idleTime");
		xml_attribute<> *maxRequests = xn->first_attribute("maxRequests");

		if (!id) {
			Warning("node \'fastcgi\' must have attribute \'id\'\n");
			return;
		}
		if (handlerNs.find(id->value()) != handlerNs.end()) {
			Warning("handler id \'%s\' already exists\n", id->value());
			return;
		}
		if (!path) {
			Warning("node \'fastcgi\' must have attribute \'path\'\n");
			return;
		}

		FcgiHandler *fc = new FcgiHandler(path->value(),
			queueLength ? atoi(queueLength->value()) : 0,
			maxInstances ? atoi(maxInstances->value()) : 0,
			idleTime ? atoi(idleTime->value()) : 30000,
			maxRequests ? atoi(maxRequests->value()) : 499);

		handlerNs.insert(make_pair(CiString(id->value()), fc));
	}
}
