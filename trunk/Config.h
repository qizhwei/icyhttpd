#ifndef _CONFIG_H
#define _CONFIG_H

#include "Utility.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

namespace Httpd
{
	class Config: NonCopyable, public rapidxml::file<>, public rapidxml::xml_document<>
	{
	public:
		Config(const char *filename);
	private:
		~Config();
	};
}

#endif
