#include "str.h"
#include "dict.h"
#include "mem.h"
#include <stdint.h>
#include <string.h>

struct str {
	// reference count to the str, or 0 if literal
	uint32_t ref_count;

	uint32_t length;
	char *buffer;
};

static dict_t g_literals;
static dict_t g_strings;

void str_init(void)
{
	dict_init(&g_literals);
	dict_init(&g_strings);
}

str_t *str_alloc(char *p)
{
	void *value;
	str_t *result;

	if (!dict_query_str(&g_strings, p, &value, 0)) {
		result = value;

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
	void *value;
	str_t *result;

	if (!dict_query_ptr(&g_literals, p, &value, 0)) {
		result = value;
	} else if (!dict_query_str(&g_strings, p, &value, 0)) {
		result = value;

		if (result->ref_count) {
			result->ref_count = 0;
			mem_free(result->buffer);
			result->buffer = p;
		}

		dict_add_ptr(&g_literals, p, result);
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

void str_free(str_t *str)
{
	if (str->ref_count && --str->ref_count == 0) {
		mem_free(str->buffer);
		mem_free(str);
	}
}
