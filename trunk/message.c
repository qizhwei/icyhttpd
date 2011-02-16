#include "message.h"
#include "dict.h"
#include "str.h"
#include <string.h>
#include <stdint.h>

static inline uint16_t uint16_parse(char *s)
{
	int result = 0;

	while (1) {
		if (*s >= '0' && *s <= '9') {
			result = result * 10 + (*s - '0');
			if (result >= UINT16_MAX)
				return UINT16_MAX;
		} else if (*s == '\0') {
			return result;
		} else {
			return UINT16_MAX;
		}
		++s;
	}
}

static inline int http_ver_parse(http_ver_t *v, char *s)
{
	char *t;

	if (s == NULL) {
		v->major = 0;
		v->minor = 9;
	} else {
		if (strncmp(s, "HTTP/", 5))
			return -1;
		s += 5;

		if ((t = strchr(s, '.')) == NULL)
			return -1;
		*t++ = '\0';

		if ((v->major = uint16_parse(s)) == UINT16_MAX)
			return -1;
		if ((v->minor = uint16_parse(t)) == UINT16_MAX)
			return -1;
	}

	return 0;
}

int request_init(request_t *r, char *line)
{
	char *req_uri;
	char *query_str;
	char *http_ver;

	if ((req_uri = strchr(line, ' ')) == NULL)
		return -1;
	*req_uri++ = '\0';

	if ((http_ver = strchr(req_uri, ' ')) != NULL)
		*http_ver++ = '\0';

	if (http_ver_parse(&r->ver, http_ver))
		return -1;

	if ((r->method = str_alloc(line)) == NULL)
		return -1;

	if ((query_str = strchr(req_uri, '?')) != NULL) {
		*query_str++ = '\0';
		if ((r->query_str = str_alloc(query_str)) == NULL) {
			str_free(r->method);
			return -1;
		}
	} else {
		r->query_str = NULL;
	}

	// TODO: what if uri begins with http://
	// TODO: uri decode
	// TODO: uri rewrite (dots and slashes)

	if ((r->req_uri = str_alloc(req_uri)) == NULL) {
		str_free(r->method);
		str_free(r->query_str);
		return -1;
	}

	dict_init(&r->headers);
	return 0;
}

void free_proc(void *key, void *value)
{
	str_free(key);
	str_free(value);
}

void request_uninit(request_t *r)
{
	str_free(r->method);
	str_free(r->req_uri);
	str_free(r->query_str);
	dict_walk(&r->headers, free_proc);
	dict_uninit(&r->headers);
}

static inline char stri_toupper(char c)
{
	if (c >= 'a' && c <= 'z')
		return c + ('A' - 'a');
	else
		return c;
}

static inline char stri_tolower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c + ('a' - 'A');
	else
		return c;
}

static inline void normalize_case(char *s)
{
	int upper = 1;

	while (*s != '\0') {
		if (upper) {
			*s = stri_toupper(*s);
			upper = 0;
		} else {
			*s = stri_tolower(*s);
		}
		if (*s++ == '-')
			upper = 1;
	}
}

int request_parse_header(request_t *r, char *line)
{
	char *p;
	str_t *key;
	void **old_value;
	str_t *value;

	// ignore the standard
	if (*line == ' ' || *line == '\t')
		return -1;

	if ((p = strchr(line, ':')) == NULL)
		return -1;
	*p++ = '\0';
	while (*p == ' ' || *p == '\t')
		++p;

	normalize_case(line);
	if ((key = str_alloc(line)) == NULL)
		return -1;

	if ((old_value = dict_query_ptr(&r->headers, key, 0)) == NULL) {
		if ((value = str_alloc(p)) == NULL) {
			str_free(key);
			return -1;
		}

		if (dict_add_ptr(&r->headers, key, value)) {
			str_free(key);
			str_free(value);
			return -1;
		}
	} else {
		*--p = ',';
		if ((value = str_concat_sp(*old_value, p)) == NULL) {
			str_free(key);
			return -1;
		}

		str_free(*old_value);
		*old_value = value;
	}

	return 0;
}
