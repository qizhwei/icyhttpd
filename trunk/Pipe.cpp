#include "Pipe.h"
#include "Types.h"
#include "Win32.h"

using namespace Httpd;

namespace
{
	// write private code here
}

namespace Httpd
{
	PipeHandle::PipeHandle(HANDLE hPipe)
		: hPipe(hPipe)
	{}

	PipeHandle::~PipeHandle()
	{
		CloseHandle(hPipe);
	}

	// write public code here
}
