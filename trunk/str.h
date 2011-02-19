#ifndef _STR_H
#define _STR_H

#include <stdint.h>

typedef struct str {
	// reference count to the str, or 0 if literal
	uint32_t ref_count;

	uint32_t length;
	char *buffer;
} str_t;

extern void str_init(void);
extern str_t *str_alloc(char *p);
extern str_t *str_uint32(uint32_t i);
extern str_t *str_concat_sp(str_t *s, char *p);
extern str_t *str_literal(char *p);
extern str_t *str_dup(str_t *s);
extern str_t *str_lower(str_t *s);
extern void str_free(str_t *s);

#endif
