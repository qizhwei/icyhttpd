#include "Dispatcher.h"
#include "Win32.h"
#include "File.h"
#include <cstdio>
#include <vector>

using namespace Httpd;
using namespace std;

namespace
{
	// test code
	void hello(void *)
	{
		vector<char> buffer(4096);
		File file(L"d:\\zzz.txt");
		UInt32 size = file.Read(&*buffer.begin(), buffer.size());
		printf("read %u bytes\n", size);
		buffer.resize(size);
		buffer.push_back('\0');
		printf("%s\n", &*buffer.begin());
	}
}

int main()
{
	Dispatcher* dispatcher = Dispatcher::Instance();
	dispatcher->Queue(hello, nullptr);

	ExitThread(0);
}
