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

void written(void *state, size_t size, int error)
{
	printf("written %d\n", size);
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
	// file = ObReferenceObjectByName(NULL, "\\FileSystem\\wwwroot\\", &FsFileObjectType);
	// TODO: FileObject.Handler, DirectoryHandler, NotExistHandler

	pool = ObReferenceObjectByName(NULL, "\\FastCgiPool\\php-cgi", NULL);
	{
		FcRequestParam param;
		param.ScriptFilename = "d:\\wwwroot\\post.php";
		param.RequestMethod = "POST";
		param.ContentLength = "7";
		param.ContentType = "application/x-www-form-urlencoded";
		request = FcBeginRequest(pool, &param);
	}
	ObDereferenceObject(pool);

	strcpy(buffer, "foo=bar");
	FcWriteRequest(request, buffer, 7, written, 0);
	FcWriteRequest(request, NULL, 0, written, 0);
	FcReadRequest(request, buffer, 1023, read, 0);
	
	while (1)
		SleepEx(-1, 1);
}
