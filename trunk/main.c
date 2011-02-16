#include "mem.h"
#include "str.h"
#include "process.h"
#include "socket.h"
#include "server.h"

int main(void)
{
	mem_init();
	str_init();
	process_init();
	socket_init();
	endpoint_create("127.0.0.1", 88);
	process_loop();
}
