#include "Endpoint.h"
#include "Node.h"
#include "FileHandler.h"
#include "Win32.h"

using namespace Httpd;

int main()
{
	new Endpoint("0.0.0.0", 88, new Node("D:\\wwwroot", new FileHandler()));
	ExitThread(0);
}
