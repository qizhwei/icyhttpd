#include "fc.h"
#include <windows.h>
#include <stdio.h>

FcPool *pool;
char buffer[1024];

void read(void *state, size_t size, int error)
{
	buffer[size] = 0;
	printf("%s\n", buffer);
}

void begin(void *state, FcRequest *request, int error)
{
	FcReadRequest(request, buffer, 1023, read, 0);
}

int main(int argc, char **argv)
{
	FcInitializeSystem();
	pool = FcCreatePool("php-cgi", 1000, 500);
	FcBeginRequest(pool, "d:\\wwwroot\\phpinfo.php", begin, 0);
	
	while (1)
		SleepEx(-1, 1);
}
