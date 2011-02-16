#include "message.h"
#include "dict.h"
#include "str.h"
#include <string.h>
#include <stdint.h>

static int http_ver_parse(http_ver_t *v, char *s)
{
	if (s == NULL) {
		v->major = 0;
		v->minor = 9;
	} else {
		// TODO: parse!
		v->major = 1;
		v->minor = 1;
	}

	return 0;
}

int request_init(request_t *r, char *line)
{
	char *req_uri = strchr(line, ' ');
	char *http_ver;

	if (req_uri == NULL)
		return -1;

	*req_uri++ = '\0';
	http_ver = strchr(req_uri, ' ');

	if (http_ver != NULL) {
		*http_ver++ = '\0';
	}

	if (http_ver_parse(&r->ver, http_ver))
		return -1;

	r->method = str_alloc(line);
	// TODO: url decode
	// TODO: url rewrite (./..)
	// TODO: seperate query string?
	r->req_uri = str_alloc(req_uri);

	dict_init(&r->headers);
	return -1;
}
