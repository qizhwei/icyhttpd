#include "fc.h"
#include <windows.h>
#include <stdio.h>

FcPool *pool;
FcRequest *request;
char buffer[1024];

void read(void *state, size_t size, int error)
{
	buffer[size] = 0;
	printf("%s\n", buffer);
}

int main(int argc, char **argv)
{
	FcInitializeSystem();
	pool = FcCreatePool("php-cgi", 0, 500);
	request = FcBeginRequest(pool, "d:\\wwwroot\\foobar.php");
	FcReadRequest(request, buffer, 1023, read, 0);
	
	while (1)
		SleepEx(-1, 1);
}
