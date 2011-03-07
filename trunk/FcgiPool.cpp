#include "FcgiPool.h"

namespace Httpd
{
	FcgiPool::FcgiPool(const std::wstring &commandLine, UInt32 idleTime, UInt32 maxRequests)
		: commandLine(commandLine), idleTime(idleTime), maxRequests(maxRequests)
	{}


}
