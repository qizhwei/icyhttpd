#include "Dispatcher.h"
#include "Win32.h"
#include <cstdio>

namespace
{
	void hello(void *)
	{
		printf("Hello world!\n");
	}
}

int main()
{
	using namespace Httpd;

	Dispatcher* dispatcher = Dispatcher::Instance();
	dispatcher->Queue(hello, nullptr);

	ExitThread(0);
}
