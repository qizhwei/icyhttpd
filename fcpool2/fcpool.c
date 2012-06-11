#include "timer.h"
#include "pool.h"
#include "inst.h"
#include "req.h"
#include "obj.h"
#include "fcpool.h"

static obj_type_t fc_request_type;

struct fc_request {
	req_t *r;
};

static void fc_request_uninit(fc_request_t *r)
{
	req_abort(r->r);
	obj_release(r->r);
}

void fc_startup(void)
{
	timer_startup();
	pool_startup();
	inst_startup();
	req_startup();
	fc_request_type.size = sizeof(fc_request_t);
	fc_request_type.uninit = (uninit_method_t *)&fc_request_uninit;
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
	return (fc_pool_t *)pool_create(cmd_line, queue_length, max_instances, idle_time, max_requests);
}

fc_request_t *fc_create_request(void)
{
	fc_request_t *r = (fc_request_t *)obj_alloc(&fc_request_type);
	if (r == NULL)
		return NULL;

	// fc request object initialization begins
	r->r = req_create();
	if (r->r == NULL) {
		obj_free(r);
		return NULL;
	}
	// fc request object initialization ends

	return r;
}

void fc_abort_request(fc_request_t *r)
{
	req_abort(r->r);
}

int fc_begin_request(fc_request_t *r, fc_pool_t *pool, fc_begin_request_callback_t *cb, void *u)
{
	return req_begin(r->r, (pool_t *)pool, (req_begin_cb_t *)cb, u);
}

int fc_read_request(fc_request_t *r, char *buf, size_t size, fc_completion_t *cb, void *u)
{
	return req_read(r->r, buf, size, (completion_t *)cb, u);
}

int fc_write_request(fc_request_t *r, const char *buf, size_t size, fc_completion_t *cb, void *u)
{
	return req_write(r->r, buf, size, (completion_t *)cb, u);
}

int fc_write_param_request(fc_request_t *r, const char *buf, size_t size, fc_completion_t *cb, void *u)
{
	return req_write_param(r->r, buf, size, (completion_t *)cb, u);
}
