#include <assert.h>
#include "internal.h"
#include "req.h"
#include "obj.h"
#include "fifo.h"
#include "inst.h"
#include "pool.h"

static obj_type_t req_type;

static void req_uninit(req_t *r)
{
	assert(r->state != REQ_PENDING);
	assert(r->state != REQ_BEGIN);
	assert(r->state != REQ_ACTIVE);
	assert(fifo_is_empty(&r->f_params));
	assert(fifo_is_empty(&r->f_stdin));
	assert(fifo_is_empty(&r->f_stdout));
}

void req_startup(void)
{
	req_type.size = sizeof(req_t);
	req_type.uninit = (uninit_method_t *)&req_uninit;
}

req_t *req_create(void)
{
	req_t *req = (req_t *)obj_alloc(&req_type);
	if (req == NULL)
		return NULL;

	// req object initialization begins
	req->state = REQ_IDLE;
	fifo_init(&req->f_params);
	fifo_init(&req->f_stdin);
	fifo_init(&req->f_stdout);
	// req object initialization ends

	return req;
}

int req_begin(req_t *r, pool_t *pool, req_begin_cb_t *cb, void *u)
{
	if (r->state != REQ_IDLE)
		return -1;

	if (pool->queue_length > 0 && pool->req_cnt >= pool->queue_length)
		return -1;

	// switch to pending state
	r->pool = (pool_t *)obj_add_ref(pool);
	obj_add_ref(r);
	list_add_tail(&pool->req_list, &r->rq_entry);
	++pool->req_cnt;
	r->begin_cb = cb;
	r->begin_u = u;
	r->state = REQ_PENDING;

	pool_marry(pool);
	return 0;
}

void req_abort(req_t *r)
{
	switch (r->state) {
	case REQ_IDLE:
		break;
	case REQ_PENDING:
		r->state = REQ_END;
		list_remove(&r->rq_entry);
		--r->pool->req_cnt;
		obj_release(r->pool);
		(*r->begin_cb)(r->begin_u, -1);
		break;
	case REQ_BEGIN:
		r->state = REQ_END;
		obj_release(r->hi);
		break;
	case REQ_ACTIVE:
		r->state = REQ_END;
		obj_release(r->hi);
		fifo_abort(&r->f_params);
		fifo_abort(&r->f_stdin);
		fifo_abort(&r->f_stdout);
		break;
	case REQ_END:
		break;
	}
}

int req_read(req_t *r, char *buf, size_t size, completion_t *cb, void *u)
{
	if (r->state != REQ_ACTIVE)
		return -1;
	return fifo_read(&r->f_stdout, buf, size, cb, u);
}

int req_write(req_t *r, const char *buf, size_t size, completion_t *cb, void *u)
{
	if (r->state != REQ_ACTIVE)
		return -1;
	return fifo_write(&r->f_stdin, buf, size, cb, u);
}

int req_write_param(req_t *r, const char *buf, size_t size, completion_t *cb, void *u)
{
	if (r->state != REQ_ACTIVE)
		return -1;
	return fifo_write(&r->f_params, buf, size, cb, u);
}
