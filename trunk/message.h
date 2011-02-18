#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "dict.h"
#include "str.h"
#include "proc.h"
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
} request_t;

typedef struct response {
	http_ver_t ver;
	int status;
	dict_t headers;
	io_proc_t *read_proc;
	io_proc_t *write_proc;
	proc_t *close_proc;
	void *object;
} response_t;

extern int request_init(request_t *r, char *line);
extern void request_uninit(request_t *r);
extern void response_uninit(response_t *r);
extern int request_parse_header(request_t *r, char *line);
extern str_t *request_alloc_ext(request_t *r);
extern str_t *request_get_header(request_t *r, str_t *key);

#endif
