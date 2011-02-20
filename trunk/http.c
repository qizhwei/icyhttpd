#include "http.h"
#include "dict.h"
#include "str.h"
#include "stri.h"
#include "time.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

static dict_t g_status;

static char *weekdays[7] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};

static char *months[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

static inline void add_status(int code, char *def_literal)
{
	dict_add_ptr(&g_status, (void *)code, str_literal(def_literal));
}

void http_init(void)
{
	dict_init(&g_status);

	add_status(100, "Continue");
	add_status(101, "Switching Protocols");
	add_status(200, "OK");
	add_status(201, "Created");
	add_status(202, "Accepted");
	add_status(203, "Non-Authoritative Information");
	add_status(204, "No Content");
	add_status(205, "Reset Content");
	add_status(206, "Partial Content");
	add_status(300, "Multiple Choices");
	add_status(301, "Moved Permanently");
	add_status(302, "Found");
	add_status(303, "See Other");
	add_status(304, "Not Modified");
	add_status(305, "Use Proxy");
	// 306 is unused and reserved
	add_status(307, "Temporary Redirect");
	add_status(400, "Bad Request");
	add_status(401, "Unauthorized");
	add_status(402, "Payment Required");
	add_status(403, "Forbidden");
	add_status(404, "Not Found");
	add_status(405, "Method Not Allowed");
	add_status(406, "Not Acceptable");
	add_status(407, "Proxy Authentication Required");
	add_status(408, "Request Timeout");
	add_status(409, "Conflict");
	add_status(410, "Gone");
	add_status(411, "Length Required");
	add_status(412, "Precondition Failed");
	add_status(413, "Request Entity Too Large");
	add_status(414, "Request-URI Too Long");
	add_status(415, "Unsupported Media Type");
	add_status(416, "Request Range Not Satisfiable");
	add_status(417, "Expectation Failed");
	add_status(500, "Internal Server Error");
	add_status(501, "Not Implemented");
	add_status(502, "Bad Gateway");
	add_status(503, "Service Unavailable");
	add_status(504, "Gateway Timeout");
	add_status(505, "HTTP Version Not Supported");
}

MAYFAIL(NULL) str_t *http_get_status(int code)
{
	void **value = dict_query_ptr(&g_status, (void *)code, 0);
	return value == NULL ? NULL : *value;
}

NOFAIL str_t *http_alloc_date(struct tm *tm)
{
	char buffer[32];

	snprintf(buffer, sizeof(buffer), "%3s, %02d %3s %04d %02d:%02d:%02d GMT",
		weekdays[tm->tm_wday], tm->tm_mday, months[tm->tm_mon], tm->tm_year + 1900,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	return str_alloc(buffer);
}

static inline MAYFAIL(UINT16_MAX) uint16_t uint16_parse(char *s)
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

static inline MAYFAIL(-1) int http_ver_parse(http_ver_t *v, char *s)
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

// uri decoding
static void uri_decode(char *uri_buf)
{
	// Search for "%"
	int i = 0, j = 0;
	char decoded_chr = 0;
	int decode_left = 0;

	while(uri_buf[i] != 0)
	{
		if(uri_buf[i] == '%')
		{
			// Initialization
			decode_left = 2;
			decoded_chr = 0;
			i ++;
			continue;
		}
		if(decode_left == 0)
		{
			// Nothing to be decoded here. Copy it
			uri_buf[j++] = uri_buf[i++];
		}
		else  // if(decode_left == 2 || decode_left == 1)
		{
			if(uri_buf[i] >= '0' && uri_buf[i] <= '9')
			{
				decoded_chr += uri_buf[i] - '0';
			}
			else if(uri_buf[i] >= 'A' && uri_buf[i] <= 'F')
			{
				decoded_chr += uri_buf[i] - 'A' + 10;
			}
			else if(uri_buf[i] >= 'a' && uri_buf[i] <= 'f')
			{
				decoded_chr += uri_buf[i] - 'a' + 10;
			}
			else
			{
				// TODO: dealing with unknown character when decoding. - by Fish
				//printf("uri_decode: unable to decode. unknown character exists.\n");
				break;
			}
			if(decode_left == 2)
			{
				decoded_chr *= 0x10;
			}
			else // decode_left == 1
			{
				uri_buf[j++] = decoded_chr;
			}
			decode_left--;
			i ++;
		}
	}
	uri_buf[j] = 0;
}

MAYFAIL(-1) int request_init(request_t *r, char *line)
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

	r->method = str_alloc(line);
	if ((query_str = strchr(req_uri, '?')) != NULL) {
		*query_str++ = '\0';
		r->query_str = str_alloc(query_str);
	} else {
		r->query_str = NULL;
	}

	uri_decode(req_uri);

	// TODO: uri rewrite (dots and slashes)
	// TODO: what if uri begins with http://
	if (*req_uri != '/') {
		str_free(r->method);
		str_free(r->query_str);
		return -1;
	}

	r->req_uri = str_alloc(req_uri);
	dict_init(&r->headers);
	return 0;
}

static inline NOFAIL int free_proc(void *u, void *key, void *value)
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

MAYFAIL(-1) int request_parse_header(request_t *r, char *line)
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
	key = str_alloc(line);

	if ((old_value = dict_query_ptr(&r->headers, key, 0)) == NULL) {
		value = str_alloc(p);
		dict_add_ptr(&r->headers, key, value);
	} else {
		// TODO: check `if and only if the entire field-value for
		// that header field is defined as a comma-separated list'
		*--p = ',';
		value = str_concat_sp(*old_value, p);
		str_free(*old_value);
		*old_value = value;
	}

	return 0;
}

NOFAIL str_t *request_alloc_ext(request_t *r)
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

MAYFAIL(NULL) str_t *request_get_header(request_t *r, str_t *key)
{
	void **value = dict_query_ptr(&r->headers, key, 0);
	return value != NULL ? *value : NULL;
}

void response_init(response_t *r, int status, io_proc_t *read_proc,
	io_proc_t *write_proc, proc_t *close_proc, void *object)
{
	r->status = status;
	dict_init(&r->headers);
	r->read_proc = read_proc;
	r->write_proc = write_proc;
	r->close_proc = close_proc;
	r->object = object;
}

void response_uninit(response_t *r)
{
	dict_walk(&r->headers, free_proc, NULL);
	dict_uninit(&r->headers);
	//r->close_proc(r->object);
}

MAYFAIL(NULL) str_t *response_get_header(response_t *r, str_t *key)
{
	void **value = dict_query_ptr(&r->headers, key, 0);
	return value != NULL ? *value : NULL;
}

void response_add_header(response_t *r, str_t *key, str_t *value)
{
	dict_add_ptr(&r->headers, str_dup(key), str_dup(value));
}
