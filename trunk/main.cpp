#include "Endpoint.h"
#include "Node.h"
#include "FileHandler.h"
#include "FcgiHandler.h"
#include "Exception.h"
#include "fcpool.h"
#include <cstdio>

using namespace Httpd;
using namespace std;

int main()
{
	fc_startup();
	Dispatcher::Instance().Queue([]() mutable->void
	{
		FcPool pool("D:\\Tools\\php-5.3.8-nts-Win32-VC9-x86\\php-cgi.exe", 32, 4, 5000, 499);
		int i = 0;
		while (true) {
			FcRequest req;
			try {
				char buffer[1024];
				UInt32 size;
				req.Begin(&pool);
				size = req.WriteParam("\x0f\x0f""SCRIPT_FILENAMEd:\\www\\test.php", 32);
				size = req.WriteParam(NULL, 0);
				size = req.Read(buffer, sizeof(buffer) - 1);
				buffer[size] = '\0';
				printf("%s", buffer);
			} catch (const SystemException &) {
				printf("exception\n");
			}
			printf(":%d:", ++i);
		}
	});
	new Endpoint("0.0.0.0", 1225, new Node("D:\\www", new FileHandler()));
	Dispatcher::Instance().ThreadEntry();
}
