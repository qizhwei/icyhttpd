#ifndef _STRI_H
#define _STRI_H

static inline int stri_isupper(char c)
{
	return c >= 'A' && c <= 'Z';
}

static inline int stri_islower(char c)
{
	return c >= 'a' && c <= 'z';
}

static inline char stri_toupper(char c)
{
	if (stri_islower(c))
		return c + ('A' - 'a');
	else
		return c;
}

static inline char stri_tolower(char c)
{
	if (stri_isupper(c))
		return c + ('a' - 'A');
	else
		return c;
}

#endif
