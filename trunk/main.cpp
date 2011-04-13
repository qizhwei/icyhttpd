#include "Endpoint.h"
#include "Node.h"
#include "FileHandler.h"
#include "Win32.h"
#include "Utility.h"

using namespace Httpd;

namespace
{
	void test(void *param)
	{
		int due = reinterpret_cast<int>(param);
		while (true) {
			printf("sleep test %d\n", due);
			Dispatcher::Instance().Sleep(due);
		}
	}
}

int main()
{
	new Endpoint("0.0.0.0", 88, new Node("F:\\iceboy\\www", new FileHandler()));
	Dispatcher::Instance().Queue(test, reinterpret_cast<void *>(1000));
	Dispatcher::Instance().ThreadEntry();
}
