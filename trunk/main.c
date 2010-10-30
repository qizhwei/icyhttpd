#include "fc.h"
#include "win32.h"
#include <stdio.h>

#define SIZE (1024)
int completed = 0;
FcPool *pool;
char buffer[SIZE];

void BeginRequestComplete(void *state, FcRequest *request, int error);
void ReadRequestComplete(void *state, size_t size, int error)
{
	FcRequest *request = state;
	int err;
	
	if (error) {
		printf("Read completed with error!\n");
		return;
	}
	
	if (size != SIZE) {
		completed += 1;
		printf("%d\n", completed);
		FcEndRequest(request);
		err = FcBeginRequest(pool, "d:\\wwwroot\\hello.php", &BeginRequestComplete, NULL);
		if (err) {
			printf("FcBeginRequest failed, completed = %d\n", completed);
		}
		return;
	}

	if (FcReadRequest(request, buffer, SIZE, ReadRequestComplete, request)) {
		printf("Read request failed!\n");
	}
}

void BeginRequestComplete(void *state, FcRequest *request, int error)
{
	if (error) {
		printf("Begin request failed!\n");
		return;
	}

	if (FcReadRequest(request, buffer, SIZE, ReadRequestComplete, request)) {
		printf("Read request failed!\n");
	}
}

int main(int argc, char **argv)
{
	FcProcess *process;
	
	pool = FcCreatePool("php-cgi", 1000, 500);
	FcBeginRequest(pool, "d:\\wwwroot\\hello.php", &BeginRequestComplete, NULL);
	
	while (1) {
		SleepEx(INFINITE, TRUE);
	}
}
