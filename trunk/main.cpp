/*#include "Endpoint.h"
#include "Node.h"
#include "FileHandler.h"
#include "Win32.h"

using namespace Httpd;

int main()
{
	new Endpoint("0.0.0.0", 88, new Node(L"F:\\iceboy\\www", new FileHandler()));
	ExitThread(0);
}*/

#include "FcgiProcess.h"
#include "Win32.h"
#include <cstdio>

using namespace Httpd;

int main()
{
	FcgiProcess *fp = new FcgiProcess(L"E:\\Tools\\php-5.3.5-nts-Win32-VC9-x86\\php-cgi.exe", 500);
	ExitThread(0);
}
