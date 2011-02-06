#include <stdio.h>
#include "process.h"
#include "server.h"

int main(void)
{
	printf("process_init = %d\n", process_init());
	printf("socket_init = %d\n", socket_init());
	printf("server_add_ip = %d\n", server_add_ip("0.0.0.0", 88));
	process_loop();
	printf("should not be here\n");
}
