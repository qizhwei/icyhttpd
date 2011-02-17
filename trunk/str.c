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

str_t *str_alloc(char *p)
{
	void **value;
	str_t *result;

	if ((value = dict_query_str(&g_strings, p, 0)) != NULL) {
		result = *value;

		if (result->ref_count)
			++result->ref_count;
	} else {
		result = mem_alloc(sizeof(str_t));
		if (result != NULL) {
			result->ref_count = 1;
			result->length = strlen(p);
			result->buffer = mem_alloc(result->length + 1);
			if (result->buffer == NULL) {
				mem_free(result);
				result = NULL;
			} else {
				memcpy(result->buffer, p, result->length + 1);
				if (dict_add_str(&g_strings, result->buffer, result)) {
					mem_free(result->buffer);
					mem_free(result);
					result = NULL;
				}
			}
		}
	}

	return result;
}

str_t *str_literal(char *p)
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

		if (dict_add_str(&g_strings, p, result)) {
			runtime_abort("Fatal error: this memory allocation must not fail.");
		} else {
			dict_add_ptr(&g_literals, p, result);
		}
	} else {
		result = mem_alloc(sizeof(str_t));
		if (result != NULL) {
			result->ref_count = 0;
			result->length = strlen(p);
			result->buffer = p;

			if (dict_add_str(&g_strings, p, result)) {
				mem_free(result);
				result = NULL;
			} else {
				dict_add_ptr(&g_literals, p, result);
			}
		}
	}

	return result;
}

static str_t *str_alloc_from_buffer(char *buffer, size_t length)
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
		if (result != NULL) {
			result->ref_count = 1;
			result->length = length;
			result->buffer = buffer;
			if (dict_add_str(&g_strings, buffer, result)) {
				mem_free(buffer);
				mem_free(result);
				result = NULL;
			}
		} else {
			mem_free(buffer);
		}
	}

	return result;
}

str_t *str_concat_sp(str_t *s, char *p)
{
	size_t p_size = strlen(p);
	char *buffer = mem_alloc(s->length + p_size + 1);

	if (buffer == NULL)
		return NULL;

	memcpy(buffer, s->buffer, s->length);
	memcpy(buffer + s->length, p, p_size + 1);
	return str_alloc_from_buffer(buffer, s->length + p_size);
}

str_t *str_dup(str_t *s)
{
	if (s != NULL && s->ref_count)
		++s->ref_count;

	return s;
}

str_t *str_lower(str_t *s)
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
	if (buffer == NULL)
		return NULL;

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
