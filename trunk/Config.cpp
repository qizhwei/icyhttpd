#include "Config.h"
#include "Exception.h"
#include "Utility.h"
#include "rapidxml.hpp"
#include "rapidxml_iterators.hpp"
#include "rapidxml_utils.hpp"
#include <cstdio>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <sstream>

using namespace Httpd;
using namespace rapidxml;
using namespace std;

namespace
{
	// types
	class RootType {};
	class StringType {};
	class IntegerType {};
	class EndpointType {};
	class NodeType {};
	class HandlerType {};

	// base and impl
	template<typename T>
	class ConfigObject;

	template<typename T>
	class ConfigContext;

	class ConfigObjectBase;

	class ConfigContextBase: NonCopyable
	{
	public:
		virtual ConfigObjectBase *CreateObject(xml_node<> *xn) = 0;

		virtual ConfigContextBase *Map(const char *name)
		{
			auto iter = types.find(name);
			return iter != types.end() ? iter->second : nullptr;
		}
	protected:
		unordered_map<CiString, ConfigContextBase *> types;
	};

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

		virtual void Fill(const char *value)
		{
			fprintf(stderr, "warning: ignoring content \'%s\'\n", value);
		}

	private:
		ConfigContextBase *context;

	protected:
		unordered_map<CiString, ConfigObjectBase *> objects;
	};

	template<typename T>
	class ConfigObjectImpl: public ConfigObjectBase
	{
	protected:
		ConfigObjectImpl()
			: ConfigObjectBase(ConfigContext<T>::Instance())
		{
		}

	public:
		void Parse(xml_node<> *xn)
		{
			for (node_iterator<char> i = xn; i != node_iterator<char>(); ++i) {
				switch (i->type()) {
				case node_element:
					{
						const char *name = i->name();
						ConfigContextBase *ctx = ConfigContext<T>::Instance()->Map(name);
						if (ctx) {
							ConfigObjectBase *object = ctx->CreateObject(&*i);
							if (ConfigContext<T>::IsNamespace()) {
								if (objects.find(name) != objects.end()) {
									fprintf(stderr, "warning: name \'%s\' already exists in context \'%s\'\n",
										name, ConfigContext<T>::Name());
								} else {
									objects.insert(make_pair(name, object));
								}
							}
						} else {
							fprintf(stderr, "warning: unknown name \'%s\' in context \'%s\'\n",
								name, ConfigContext<T>::Name());
						}
					}
					break;
				default:
					fprintf(stderr, "warning: ignoring node \'%s\'\n", i->name());
				}
			}
		}
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

	class ConfigNamespace: NonCopyable
	{
	public:
		static ConfigNamespace &Instance()
		{
			static ConfigNamespace cn;
			return cn;
		}

		void Add(const char *id, ConfigObjectBase *object)
		{
			if (map.find(id) != map.end()) {
				fprintf(stderr, "warning: id \'%s\' already exists\n", id);
			} else {
				map.insert(make_pair(id, object));
			}
		}

		ConfigObjectBase *Find(const char *ref)
		{
			auto iter = map.find(ref);
			return iter != map.end() ? iter->second : nullptr;
		}

	private:
		unordered_map<CiString, ConfigObjectBase *> map;
	};

	template<typename T>
	class ConfigContextImpl: public ConfigContextBase
	{
	public:
		virtual ConfigObjectBase *CreateObject(xml_node<> *xn)
		{
			xml_attribute<> *ref = xn->first_attribute("ref");
			xml_attribute<> *id = xn->first_attribute("id");
			ConfigObjectBase *result;

			if (ref) {
				result = new ConfigNameReference(ref->value(), this);
			} else {
				ConfigObject<T> *object = new ConfigObject<T>;
				object->Parse(xn);
				result = object;
			}

			if (id) {
				ConfigNamespace::Instance().Add(id->value(), result);
			}

			return result;
		}

		static ConfigContext<T> *Instance()
		{
			static ConfigContext<T> *inst(new ConfigContext<T>);
			return inst;
		}
	};

	// instantiation
	template<>
	class ConfigContext<RootType>: public ConfigContextImpl<RootType>
	{
	public:
		ConfigContext();
		static const char *Name() { return "root"; }
		static bool IsNamespace() { return false; }
	};

	template<>
	class ConfigContext<StringType>: public ConfigContextImpl<StringType>
	{
	public:
		static const char *Name() { return "string"; }
		static bool IsNamespace() { return false; }
	};
	
	template<>
	class ConfigContext<IntegerType>: public ConfigContextImpl<IntegerType>
	{
	public:
		static const char *Name() { return "int"; }
		static bool IsNamespace() { return false; }
	};

	template<>
	class ConfigContext<EndpointType>: public ConfigContextImpl<EndpointType>
	{
	public:
		ConfigContext();
		static const char *Name() { return "endpoint"; }
		static bool IsNamespace() { return true; }
	};

	template<>
	class ConfigContext<NodeType>: public ConfigContextImpl<NodeType>
	{
	public:
		static const char *Name() { return "node"; }
		static bool IsNamespace() { return true; }
	};

	template<>
	class ConfigContext<HandlerType>: public ConfigContextImpl<HandlerType>
	{
	public:
		static const char *Name() { return "handler"; }
		static bool IsNamespace() { return true; }
	};

	ConfigContext<RootType>::ConfigContext()
	{
		types.insert(make_pair("string", ConfigContext<StringType>::Instance()));
		types.insert(make_pair("int", ConfigContext<IntegerType>::Instance()));
		types.insert(make_pair("endpoint", ConfigContext<EndpointType>::Instance()));
		types.insert(make_pair("node", ConfigContext<NodeType>::Instance()));
		types.insert(make_pair("handler", ConfigContext<HandlerType>::Instance()));
	}

	ConfigContext<EndpointType>::ConfigContext()
	{
		types.insert(make_pair("ip", ConfigContext<StringType>::Instance()));
		types.insert(make_pair("port", ConfigContext<IntegerType>::Instance()));
		types.insert(make_pair("defaultNode", ConfigContext<NodeType>::Instance()));
	}

	template<>
	class ConfigObject<RootType>: public ConfigObjectImpl<RootType>
	{
	};

	template<>
	class ConfigObject<StringType>: public ConfigObjectImpl<StringType>
	{
	public:
		void Parse(xml_node<> *xn)
		{
			s = xn->value();
			ConfigObjectImpl::Parse(xn);
		}

	private:
		const char *s;
	};

	template<>
	class ConfigObject<IntegerType>: public ConfigObjectImpl<IntegerType>
	{
	public:
		void Parse(xml_node<> *xn)
		{
			const char *value = xn->value();
			istringstream iss(value);
			iss >> i;
			if (!iss.eof()) {
				fprintf(stderr, "warning: cannot parse integer from content \'%s\'\n", value);
				i = 0;
			}
			ConfigObjectImpl::Parse(xn);
		}
	private:
		int i;
	};

	template<>
	class ConfigObject<EndpointType>: public ConfigObjectImpl<EndpointType>
	{
	};

	template<>
	class ConfigObject<NodeType>: public ConfigObjectImpl<NodeType>
	{
	};

	template<>
	class ConfigObject<HandlerType>: public ConfigObjectImpl<HandlerType>
	{
	};
}

namespace Httpd
{
	Config::Config(const char *filename)
		: file(filename)
	{
		// syntaticly parsing config file
		parse<parse_no_data_nodes | parse_trim_whitespace>(data());

		xml_node<> *root = this->first_node("icyhttpd");
		if (root == nullptr) {
			throw exception("node \'icyhttpd\' not found in configuration file");
		}

		ConfigContext<RootType>::Instance()->CreateObject(root);
	}
}
