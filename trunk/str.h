#ifndef _STR_H
#define _STR_H

typedef struct str str_t;

extern void str_init(void);
extern str_t *str_alloc(char *p);
extern str_t *str_literal(char *p);
extern void str_free(str_t *str);

#endif
