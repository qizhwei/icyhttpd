#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "dict.h"
#include "str.h"
#include <stdint.h>

typedef struct http_ver {
	uint16_t major;
	uint16_t minor;
} http_ver_t;

typedef struct request {
	str_t *method;
	str_t *req_uri;
	str_t *query_str;
	http_ver_t ver;
	dict_t headers;
	// TODO: read_func / close_func / context
} request_t;

typedef struct response {
	http_ver_t ver;
	int status;
	dict_t headers;
	// TODO: read_func / close_func / context
} response_t;

extern int request_init(request_t *r, char *line);
extern void request_uninit(request_t *r);
extern int request_parse_header(request_t *r, char *line);

#endif
