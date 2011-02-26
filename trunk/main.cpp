#include "Endpoint.h"
#include "Node.h"
#include "EchoHandler.h"
#include "Win32.h"

using namespace Httpd;

int main()
{
	new Endpoint("127.0.0.1", 88, new Node("D:\\wwwroot", new EchoHandler()));
	ExitThread(0);
}
