#include "echo_handler.h"

typedef struct echo_handler {
	handler_t handler;
} echo_handler_t;

static echo_handler_t handler;
handler_t *echo_handler;

static int handle_proc(handler_t *handler, request_t *request, response_t *response)
{
	// TODO: write an echo handler here
}

void echo_handler_init(void)
{
	handler.handler.handle_proc = handle_proc;
	echo_handler = &handler.handler;
}
