#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "dict.h"
#include "str.h"

#define HTTP_MAX_LINE (8192)

typedef struct http_ver {
	unsigned short major;
	unsigned short minor;
} http_ver_t;

typedef struct request {
	str_t *method;
	str_t *req_uri;
	http_ver_t ver;
	dict_t headers;
	// TODO: read_func / close_func / context
} request_t;

typedef struct response {
	http_ver_t ver;
	unsigned short status;
	unsigned char header_eof;
	unsigned char keep_alive;
	dict_t headers;
	// TODO: read_func / close_func / context
} response_t;

extern int request_init(request_t *r, char *line);

#endif
