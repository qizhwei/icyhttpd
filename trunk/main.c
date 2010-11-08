#include "fc.h"
#include "fs.h"
#include "ob.h"
#include <windows.h>
#include <stdio.h>

FsFileObject *file;
FcPool *pool;
FcRequest *request;
char buffer[1024];

void read(void *state, size_t size, int error)
{
	buffer[size] = 0;
	printf("%s\n", buffer);
}

void init(void)
{
	ObInitializeSystem();
	FcInitializeSystem();
	FsInitializeSystem();
	ObDereferenceObject(FcCreatePool("php-cgi", "php-cgi", 0, 500));
	ObDereferenceObject(FsCreateVirtualDirectory("wwwroot", "d:\\wwwroot"));
}

int main(int argc, char **argv)
{
	init();
	file = ObReferenceObjectByName(NULL, "\\FileSystem\\wwwroot\\foobar.php", &FsFileObjectType);
	pool = ObReferenceObjectByName(NULL, "\\FastCgiPool\\php-cgi", NULL);
	// TODO: FileObject.Handler, DirectoryHandler, NotExistHandler
	request = FcBeginRequest(pool, "d:\\wwwroot\\foobar.php");
	ObDereferenceObject(pool);

	FcReadRequest(request, buffer, 1023, read, 0);
	
	while (1)
		SleepEx(-1, 1);
}
