#include <stdio.h>
#include "dict.h"
#include "mem.h"

int main(void)
{
	dict_t dict;
	int i;
	mem_init();
	dict_init(&dict);

	for (i = 0; i < 70514; ++i)
		if (dict_add_ptr(&dict, (void *)i, (void *)i))
			printf("err\n");
}
