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
	Dispatcher::Instance().ThreadEntry();
}
