#include "Endpoint.h"
#include "Node.h"
#include "FileHandler.h"
#include "Win32.h"
#include "Utility.h"
#include <memory>

using namespace Httpd;
using namespace std;

namespace
{
	shared_ptr<WakeToken> wt(new WakeToken());

	void test1(void *)
	{
		while (true) {
			printf("sleep test %d\n", (int)Dispatcher::Instance().Sleep(1000, wt));
		}
	}

	void test2(void *)
	{
		while (true) {
			printf("wake test: %d\n", (int)wt->Wake());
			Dispatcher::Instance().Sleep(2500);
		}
	}
}

int main()
{
	new Endpoint("0.0.0.0", 88, new Node("F:\\iceboy\\www", new FileHandler()));
	Dispatcher::Instance().Queue(test1, nullptr);
	Dispatcher::Instance().Queue(test2, nullptr);
	Dispatcher::Instance().ThreadEntry();
}
