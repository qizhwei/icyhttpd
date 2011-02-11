#include <stdio.h>
#include "mem.h"
#include "str.h"

int main(void)
{
	char x[] = "hello";

	mem_init();
	str_init();

	printf("%d\n", str_alloc(x) == str_alloc(x));
	x[0] = '\0';
	printf("%d\n", str_alloc("hello") == str_literal("hello"));
	printf("%d\n", str_literal("hello") == str_literal("hello"));
}
