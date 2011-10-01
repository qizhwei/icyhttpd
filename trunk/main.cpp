#include "Endpoint.h"
#include "Node.h"
#include "FileHandler.h"

extern "C" {
	#include "fc.h"
	#include "ob.h"
}

using namespace Httpd;
using namespace std;

FcPool *pool;
FcRequest *request;
char buffer[1024];

void read(void *state, size_t size, int error)
{
	buffer[size] = 0;
	printf("%s\n", buffer);
}

void written(void *state, size_t size, int error)
{
	printf("written %d\n", size);
}

int main()
{
	Dispatcher::Instance().Queue([]()->void
	{
		Dispatcher::Instance().InvokeApc([]()->void *
		{
			ObInitializeSystem();
			FcInitializeSystem();
			ObDereferenceObject(FcCreatePool("php-cgi", "D:\\Tools\\php-5.3.8-nts-Win32-VC9-x86\\php-cgi.exe", 0, 500));
			pool = (FcPool *)ObReferenceObjectByName(NULL, "\\FastCgiPool\\php-cgi", NULL);
			{
				FcRequestParam param;
				param.ScriptFilename = "d:\\wwwroot\\post.php";
				param.RequestMethod = "POST";
				param.ContentLength = "7";
				param.ContentType = "application/x-www-form-urlencoded";
				request = FcBeginRequest(pool, &param);
			}
			ObDereferenceObject(pool);

			strcpy(buffer, "foo=bar");
			FcWriteRequest(request, buffer, 7, written, 0);
			FcWriteRequest(request, NULL, 0, written, 0);
			FcReadRequest(request, buffer, 1023, read, 0);
			return NULL;
		});
	});
	new Endpoint("0.0.0.0", 1225, new Node("D:\\www", new FileHandler()));
	Dispatcher::Instance().ThreadEntry();
}
