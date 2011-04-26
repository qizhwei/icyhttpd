#include "Endpoint.h"
#include "Node.h"
#include "FileHandler.h"

using namespace Httpd;
using namespace std;

int main()
{
	new Endpoint("0.0.0.0", 1225, new Node("D:\\www", new FileHandler()));
	Dispatcher::Instance().ThreadEntry();
}
