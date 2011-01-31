#include <stdio.h>
#include "shrimp.h"
#include "server.h"

int main(void)
{
	printf("shrimp_init = %d\n", shrimp_init());
	printf("socket_init = %d\n", socket_init());
	printf("server_add_ip = %d\n", server_add_ip("0.0.0.0", 88));
	shrimp_loop();
	printf("should not be here\n");
}
