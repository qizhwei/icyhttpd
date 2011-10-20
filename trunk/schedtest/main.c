#include "sched.h"
#include <assert.h>
#include <stdio.h>

void *worker(void *param)
{
	printf("in worker %d\n", (int)param);
	return (void *)123;
}

int main(void)
{
	sched_worker_t *w;
	assert(sched_startup() == 0);
	w = sched_begin(SCHED_ENVIRON_ST, &worker, (void *)70514);
	printf("join %d\n", (int)sched_join(w));
	sched_loop_forever();
}
