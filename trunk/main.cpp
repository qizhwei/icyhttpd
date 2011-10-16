#include "fcpool.h"
#include "Config.h"
#include "Dispatcher.h"
#include <stdexcept>
#include <cstdio>
#include <cstdlib>

using namespace Httpd;
using namespace std;

int main()
{
	Dispatcher &d = Dispatcher::Instance();

	// subsystem(s) startup
	fc_startup();

	// load configuration file
	d.Queue([]()->void {
		try {
			new Config("config.xml");
		} catch (exception &e) {
			puts(e.what());
			exit(1);
		}
	});

	// thread entry
	d.ThreadEntry();
}
