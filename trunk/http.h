#ifndef _HTTP_H
#define _HTTP_H

#include "runtime.h"
#include "dict.h"
#include "str.h"
#include "time.h"
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
	int status;
	dict_t headers;
	io_proc_t *read_proc;
	io_proc_t *write_proc;
	proc_t *close_proc;
	void *object;
} response_t;

extern void http_init(void);
extern MAYFAIL(NULL) str_t *http_get_status(int code);
extern NOFAIL str_t *http_alloc_date(void);
extern MAYFAIL(-1) int request_init(request_t *r, char *line);
extern void request_uninit(request_t *r);
extern MAYFAIL(-1) int request_parse_header(request_t *r, char *line);
extern NOFAIL str_t *request_alloc_ext(request_t *r);
extern MAYFAIL(NULL) str_t *request_get_header(request_t *r, str_t *key);
extern void response_init(response_t *r, int status, io_proc_t *read_proc,
	io_proc_t *write_proc, proc_t *close_proc, void *object);
extern void response_uninit(response_t *r);
extern MAYFAIL(NULL) str_t *response_get_header(response_t *r, str_t *key);
extern void response_add_header(response_t *r, str_t *key, str_t *value);

#endif
