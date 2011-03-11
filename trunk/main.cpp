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
#include "FcgiSession.h"
#include "Win32.h"
#include <cstdio>

using namespace Httpd;

void hello(void *)
{
	FcgiProcess *fp = FcgiProcess::Create(L"E:\\Tools\\php-5.3.5-nts-Win32-VC9-x86\\php-cgi.exe", 500);
	FcgiSession fs(*fp);
	fs.WriteParam("SCRIPT_FILENAME", "d:\\wwwroot\\echo.php");
	fs.WriteParam("CONTENT_LENGTH", "11");
	fs.WriteParam("REQUEST_METHOD", "POST");
	fs.WriteParam("CONTENT_TYPE", "application/x-www-form-urlencoded");
	fs.CloseParam();
	fs.Write("hello=world", 11);
	fs.CloseStdin();

	char buffer[4097];
	UInt32 size;
	while ((size = fs.Read(buffer, 4096)) != 0) {
		buffer[size] = '\0';
		printf("%s", buffer);
	}
	printf("\n---\nhas error: %d\n", fs.HasError());
	printf("keep alive: %d\n", fs.KeepAlive());
	delete fp;
}

int main()
{
	Dispatcher::Instance().Queue(hello, nullptr);
	ExitThread(0);
}
