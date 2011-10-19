#include "fcpool.h"
#include "Loader.h"
#include "Dispatcher.h"

using namespace Httpd;

int main()
{
	// subsystem(s) startup
	fc_startup();

	// load configuration file
	Loader("config.xml").LoadConfig();

	// thread entry
	Dispatcher::Instance().ThreadEntry();
}
