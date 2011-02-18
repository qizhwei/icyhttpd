#include "mem.h"
#include "str.h"
#include "message.h"
#include "process.h"
#include "socket.h"
#include "endpoint.h"
#include "echo_handler.h"

int main(void)
{
	mem_init();
	str_init();
	process_init();
	socket_init();
	http_init();
	echo_handler_init();
	endpoint_create("127.0.0.1", 88, node_create(str_literal("D:\\wwwroot"), echo_handler));
	process_loop();
}
