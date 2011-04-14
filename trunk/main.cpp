#include "Endpoint.h"
#include "Node.h"
#include "FileHandler.h"
#include "Win32.h"
#include "Utility.h"
#include <memory>
#include <cstdio>

using namespace Httpd;
using namespace std;

int main()
{
	WakeToken wt;

	new Endpoint("0.0.0.0", 88, new Node("F:\\iceboy\\www", new FileHandler()));
	Dispatcher::Instance().Queue([&wt](){
		while (true) {
			printf("sleep test %d\n", (int)Dispatcher::Instance().Sleep(1000, wt));
		}
	});
	Dispatcher::Instance().Queue([&wt](){
		while (true) {
			printf("wake test: %d\n", (int)wt.Wake());
			Dispatcher::Instance().Sleep(1500);
		}
	});
	Dispatcher::Instance().ThreadEntry();
}
