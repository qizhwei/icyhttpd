#include "Endpoint.h"
#include "Types.h"
#include <unordered_map>
#include <string>

using namespace std;

namespace Httpd
{
	Endpoint::Endpoint(const char *ip, UInt16 port, Node *defaultNode)
		: defaultNode(defaultNode)
	{
		socket.BindIP(ip, port);
		socket.Listen();
	}
}
