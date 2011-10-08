#include "timer.h"
#include "pool.h"
#include "inst.h"
#include "req.h"
#include "obj.h"
#include "fcpool.h"

int fc_startup(void)
{
	if (timer_startup())
		return -1;
	if (pool_startup())
		return -1;
	if (inst_startup())
		return -1;
	if (req_startup())
		return -1;

	return 0;
}

void fc_close(void *object)
{
	obj_release(object);
}

void *fc_duplicate(void *object)
{
	return obj_add_ref(object);
}

fc_pool_t *fc_create_pool(const char *cmd_line, int queue_length,
	int max_instances, int idle_time, int max_requests)
{
	return pool_create(cmd_line, queue_length, max_instances, idle_time, max_requests);
}

fc_request_t *fc_create_request(void)
{
	return req_create();
}

void fc_abort_request(fc_request_t *r)
{
	req_abort((req_t *)r);
}

int fc_begin_request(fc_request_t *r, fc_pool_t *pool, fc_begin_request_callback_t *cb, void *u)
{
	return req_begin((req_t *)r, (pool_t *)pool, (req_begin_cb_t *)cb, u);
}

int fc_read_request(fc_request_t *r, char *buf, size_t size, fc_completion_t *cb, void *u)
{
	return req_read((req_t *)r, buf, size, (completion_t *)cb, u);
}

int fc_write_request(fc_request_t *r, const char *buf, size_t size, fc_completion_t *cb, void *u)
{
	return req_write((req_t *)r, buf, size, (completion_t *)cb, u);
}

int fc_write_param_request(fc_request_t *r, const char *buf, size_t size, fc_completion_t *cb, void *u)
{
	return req_write_param((req_t *)r, buf, size, (completion_t *)cb, u);
}
