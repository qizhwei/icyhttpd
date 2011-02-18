#include "message.h"
#include "dict.h"
#include "str.h"
#include "stri.h"
#include <string.h>
#include <stdint.h>

static dict_t g_status;

static int add_status(int code, char *def_literal)
{
	str_t *str;

	if ((str = str_literal(def_literal)) == NULL)
		return -1;
	if (dict_add_ptr(&g_status, (void *)code, str))
		return -1;

	return 0;
}

int http_init(void)
{
	dict_init(&g_status);

	if (add_status(100, "Continue")
		|| add_status(101, "Switching Protocols")
		|| add_status(200, "OK")
		|| add_status(201, "Created")
		|| add_status(202, "Accepted")
		|| add_status(203, "Non-Authoritative Information")
		|| add_status(204, "No Content")
		|| add_status(205, "Reset Content")
		|| add_status(206, "Partial Content")
		|| add_status(300, "Multiple Choices")
		|| add_status(301, "Moved Permanently")
		|| add_status(302, "Found")
		|| add_status(303, "See Other")
		|| add_status(304, "Not Modified")
		|| add_status(305, "Use Proxy")
		// 306 is unused and reserved
		|| add_status(307, "Temporary Redirect")
		|| add_status(400, "Bad Request")
		|| add_status(401, "Unauthorized")
		|| add_status(402, "Payment Required")
		|| add_status(403, "Forbidden")
		|| add_status(404, "Not Found")
		|| add_status(405, "Method Not Allowed")
		|| add_status(406, "Not Acceptable")
		|| add_status(407, "Proxy Authentication Required")
		|| add_status(408, "Request Timeout")
		|| add_status(409, "Conflict")
		|| add_status(410, "Gone")
		|| add_status(411, "Length Required")
		|| add_status(412, "Precondition Failed")
		|| add_status(413, "Request Entity Too Large")
		|| add_status(414, "Request-URI Too Long")
		|| add_status(415, "Unsupported Media Type")
		|| add_status(416, "Request Range Not Satisfiable")
		|| add_status(417, "Expectation Failed")
		|| add_status(500, "Internal Server Error")
		|| add_status(501, "Not Implemented")
		|| add_status(502, "Bad Gateway")
		|| add_status(503, "Service Unavailable")
		|| add_status(504, "Gateway Timeout")
		|| add_status(505, "HTTP Version Not Supported"))
		return -1;

	return 0;
}

str_t *http_get_status(int code)
{
	void **value = dict_query_ptr(&g_status, (void *)code, 0);
	return value == NULL ? NULL : *value;
}

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

	// TODO: uri decode
	// TODO: uri rewrite (dots and slashes)
	// TODO: what if uri begins with http://
	if (*req_uri != '/') {
		str_free(r->method);
		str_free(r->query_str);
		return -1;
	}

	if ((r->req_uri = str_alloc(req_uri)) == NULL) {
		str_free(r->method);
		str_free(r->query_str);
		return -1;
	}

	dict_init(&r->headers);
	return 0;
}

static inline int free_proc(void *u, void *key, void *value)
{
	str_free(key);
	str_free(value);
	return 0;
}

void request_uninit(request_t *r)
{
	str_free(r->method);
	str_free(r->req_uri);
	str_free(r->query_str);
	dict_walk(&r->headers, free_proc, NULL);
	dict_uninit(&r->headers);
}

void response_uninit(response_t *r)
{
	dict_walk(&r->headers, free_proc, NULL);
	r->close_proc(r->object);
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
		// TODO: check `if and only if the entire field-value for
		// that header field is defined as a comma-separated list'
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

str_t *request_alloc_ext(request_t *r)
{
	char *p = r->req_uri->buffer + r->req_uri->length;

	while (1) {
		--p;
		if (*p == '/')
			return str_literal(".");
		else if (*p == '.')
			return str_alloc(p);
	}
}

str_t *request_get_header(request_t *r, str_t *key)
{
	void **value = dict_query_ptr(&r->headers, key, 0);
	return value != NULL ? *value : NULL;
}
