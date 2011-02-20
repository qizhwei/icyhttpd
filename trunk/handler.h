#ifndef _HANDLER_H
#define _HANDLER_H

#include "http.h"
#include "runtime.h"

typedef struct handler handler_t;
typedef MAYFAIL(-1) int handle_proc_t(handler_t *handler, request_t *request, response_t *response);

struct handler {
	handle_proc_t *handle_proc;
};

#endif
