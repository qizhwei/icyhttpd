#include "Config.h"
#include "Exception.h"
#include "Utility.h"
#include "rapidxml.hpp"
#include "rapidxml_iterators.hpp"
#include "rapidxml_utils.hpp"
#include <cstdio>
#include <unordered_set>
#include <unordered_map>

using namespace Httpd;
using namespace rapidxml;
using namespace std;

namespace
{
	// types
	class RootType
	{
	public:
		static const char *Name() { return "root"; }
	};

	class StringType
	{
	public:
		static const char *Name() { return "string"; }
	};

	class IntegerType
	{
	public:
		static const char *Name() { return "int"; }
	};

	class EndpointType
	{
	public:
		static const char *Name() { return "endpoint"; }
	};

	class NodeType
	{
	public:
		static const char *Name() { return "node"; }
	};

	class HandlerType
	{
	public:
		static const char *Name() { return "handler"; }
	};

	// base and impl
	template<typename T>
	class ConfigObject;

	template<typename T>
	class ConfigContext;

	class ConfigContextBase;

	class ConfigObjectBase: NonCopyable
	{
	protected:
		ConfigObjectBase(ConfigContextBase *context)
			: context(context)
		{
		}

		ConfigContextBase *Context()
		{
			return context;
		}

	private:
		ConfigContextBase *context;
	};

	template<typename T>
	class ConfigObjectImpl: public ConfigObjectBase
	{
	protected:
		ConfigObjectImpl()
			: ConfigObjectBase(ConfigContext<T>::Instance())
		{
		}
	};

	class ConfigContextBase: NonCopyable
	{
	public:
		virtual ConfigObjectBase *CreateObject(xml_node<> *xn) = 0;

		virtual ConfigContextBase *Map(const char *name)
		{
			auto iter = types.find(name);
			return iter != types.end() ? iter->second : nullptr;
		}

		virtual const char *Type() = 0;
	protected:
		unordered_map<CiString, ConfigContextBase *> types;
	};

	class ConfigNameReference: public ConfigObjectBase
	{
	public:
		ConfigNameReference(const char *ref, ConfigContextBase *context)
			: ref(ref), ConfigObjectBase(context)
		{
		}

	private:
		const char *ref;
	};

	template<typename T>
	class ConfigContextImpl: public ConfigContextBase
	{
	public:
		virtual ConfigObjectBase *CreateObject(xml_node<> *xn)
		{
			xml_attribute<> *ref = xn->first_attribute("ref");
			if (ref) {
				return new ConfigNameReference(ref->value(), this);
			} else {
				return new ConfigObject<T>(xn);
			}
		}

		static ConfigContext<T> *Instance()
		{
			static ConfigContext<T> *inst(new ConfigContext<T>);
			return inst;
		}

		virtual const char *Type()
		{
			return T::Name();
		}
	};

	// instantiation
	template<>
	class ConfigContext<RootType>: public ConfigContextImpl<RootType>
	{
	public:
		ConfigContext();
	};

	template<>
	class ConfigContext<StringType>: public ConfigContextImpl<StringType>
	{
	};
	
	template<>
	class ConfigContext<IntegerType>: public ConfigContextImpl<IntegerType>
	{
	};

	template<>
	class ConfigContext<EndpointType>: public ConfigContextImpl<EndpointType>
	{
	};

	template<>
	class ConfigContext<NodeType>: public ConfigContextImpl<NodeType>
	{
	};

	template<>
	class ConfigContext<HandlerType>: public ConfigContextImpl<HandlerType>
	{
	};

	ConfigContext<RootType>::ConfigContext()
	{
		types.insert(make_pair("string", ConfigContext<StringType>::Instance()));
		types.insert(make_pair("int", ConfigContext<IntegerType>::Instance()));
		types.insert(make_pair("endpoint", ConfigContext<EndpointType>::Instance()));
		types.insert(make_pair("node", ConfigContext<NodeType>::Instance()));
		types.insert(make_pair("handler", ConfigContext<HandlerType>::Instance()));
	}

	template<>
	class ConfigObject<RootType>: public ConfigObjectImpl<RootType>
	{
	public:
		ConfigObject(xml_node<> *xn)
		{
			// TODO
			for (node_iterator<char> i = xn; i != node_iterator<char>(); ++i) {
				printf("%s is type of %s\n", i->name(), Context()->Map(i->name())->Type());
			}
		}
	};

	template<>
	class ConfigObject<StringType>: public ConfigObjectImpl<StringType>
	{
	public:
		ConfigObject(xml_node<> *xn)
		{
		}
	};

	template<>
	class ConfigObject<IntegerType>: public ConfigObjectImpl<IntegerType>
	{
	public:
		ConfigObject(xml_node<> *xn)
		{
		}
	};

	template<>
	class ConfigObject<EndpointType>: public ConfigObjectImpl<EndpointType>
	{
	public:
		ConfigObject(xml_node<> *xn)
		{
		}
	};

	template<>
	class ConfigObject<NodeType>: public ConfigObjectImpl<NodeType>
	{
	public:
		ConfigObject(xml_node<> *xn)
		{
		}
	};

	template<>
	class ConfigObject<HandlerType>: public ConfigObjectImpl<HandlerType>
	{
	public:
		ConfigObject(xml_node<> *xn)
		{
		}
	};
}

namespace Httpd
{
	Config::Config(const char *filename)
		: file(filename)
	{
		// syntaticly parsing config file
		parse<0>(data());

		xml_node<> *root = this->first_node("icyhttpd");
		if (root == nullptr) {
			throw exception("node \'icyhttpd\' not found in configuration file");
		}

		ConfigContext<RootType>::Instance()->CreateObject(root);
	}
}
