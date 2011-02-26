#include "Dispatcher.h"
#include "Win32.h"
#include "Socket.h"
#include <cstdio>
#include <vector>

using namespace Httpd;
using namespace std;

namespace
{
	// test code
	void hello(void *)
	{
		Socket s;
		s.BindIP(88);
		s.Listen();
		while (true) {
			char buffer[4096];
			Socket a;
			s.Accept(a);
			printf("connection established\n");
			while (true) {
				UInt32 x = a.Read(buffer, 4096);
				if (x == 0)
					break;
				printf("received %u bytes\n", x);
			}
			printf("connection broken\n");
		}
	}
}

int main()
{
	Dispatcher* dispatcher = Dispatcher::Instance();
	dispatcher->Queue(hello, nullptr);

	ExitThread(0);
}
