#include "str.h"
#include "dict.h"
#include "mem.h"
#include "stri.h"
#include <string.h>

static dict_t g_literals;
static dict_t g_strings;

void str_init(void)
{
	dict_init(&g_literals);
	dict_init(&g_strings);
}

NOFAIL str_t *str_alloc(char *p)
{
	void **value;
	str_t *result;

	if ((value = dict_query_str(&g_strings, p, 0)) != NULL) {
		result = *value;

		if (result->ref_count)
			++result->ref_count;
	} else {
		result = mem_alloc(sizeof(str_t));
		result->ref_count = 1;
		result->length = strlen(p);
		result->buffer = mem_alloc(result->length + 1);
		memcpy(result->buffer, p, result->length + 1);
		dict_add_str(&g_strings, result->buffer, result);
	}

	return result;
}

NOFAIL str_t *str_uint32(uint32_t i)
{
	char buffer[16];
	char *p = buffer + sizeof(buffer);

	*--p = '\0';
	while (i) {
		*--p = '0' + (i % 10);
		i /= 10;
	}

	if (*p == '\0')
		*--p = '0';

	return str_alloc(p);
}

NOFAIL str_t *str_literal(char *p)
{
	void **value;
	str_t *result;

	if ((value = dict_query_ptr(&g_literals, p, 0)) != NULL) {
		result = *value;
	} else if ((value = dict_query_str(&g_strings, p, 1)) != NULL) {
		result = *value;
		if (result->ref_count) {
			result->ref_count = 0;
			mem_free(result->buffer);
			result->buffer = p;
		}
		dict_add_str(&g_strings, p, result);
		dict_add_ptr(&g_literals, p, result);
	} else {
		result = mem_alloc(sizeof(str_t));
		result->ref_count = 0;
		result->length = strlen(p);
		result->buffer = p;
		dict_add_str(&g_strings, p, result);
		dict_add_ptr(&g_literals, p, result);
	}

	return result;
}

static NOFAIL str_t *str_alloc_from_buffer(char *buffer, size_t length)
{
	void **value;
	str_t *result;

	if ((value = dict_query_str(&g_strings, buffer, 0)) != NULL) {
		mem_free(buffer);
		result = *value;
		if (result->ref_count)
			++result->ref_count;
	} else {
		result = mem_alloc(sizeof(str_t));
		result->ref_count = 1;
		result->length = length;
		result->buffer = buffer;
		dict_add_str(&g_strings, buffer, result);
	}

	return result;
}

NOFAIL str_t *str_concat(str_t *s, str_t *t)
{
	char *buffer = mem_alloc(s->length + t->length + 1);
	memcpy(buffer, s->buffer, s->length);
	memcpy(buffer + s->length, t->buffer, t->length + 1);
	return str_alloc_from_buffer(buffer, s->length + t->length);
}

NOFAIL str_t *str_concat_sp(str_t *s, char *p)
{
	size_t p_size = strlen(p);
	char *buffer = mem_alloc(s->length + p_size + 1);
	memcpy(buffer, s->buffer, s->length);
	memcpy(buffer + s->length, p, p_size + 1);
	return str_alloc_from_buffer(buffer, s->length + p_size);
}

NOFAIL str_t *str_dup(str_t *s)
{
	if (s != NULL && s->ref_count)
		++s->ref_count;

	return s;
}

NOFAIL str_t *str_lower(str_t *s)
{
	int i = 0;
	char *buffer;

	while (1) {
		if (s->buffer[i] == '\0') {
			++s->ref_count;
			return s;
		}
		if (stri_isupper(s->buffer[i]))
			break;
		++i;
	}
	buffer = mem_alloc(s->length + 1);
	memcpy(buffer, s->buffer, i);
	do
		buffer[i] = stri_tolower(s->buffer[i]);
	while (buffer[i++] != '\0');

	return str_alloc_from_buffer(buffer, i);
}

void str_free(str_t *s)
{
	if (s != NULL && s->ref_count && --s->ref_count == 0) {
		dict_query_str(&g_strings, s->buffer, 1);
		mem_free(s->buffer);
		mem_free(s);
	}
}
