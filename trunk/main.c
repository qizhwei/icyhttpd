#include <stdio.h>
#include "dict.h"
#include "mem.h"

int main(void)
{
	dict_t dict;
	int i;
	void *value;

	mem_init();
	dict_init(&dict);

	if (dict_add_str(&dict, "HIso1", (void *)70514))
		printf("err\n");

	printf("%d\n", dict_query_stri(&dict, "hiso1", &value, 1));
	printf("%d\n", (int)value);
	printf("%d\n", dict_query_stri(&dict, "hiso1", &value, 1));
	printf("%d\n", (int)value);
}
