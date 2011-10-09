#include <assert.h>
#include <windows.h>
#include "internal.h"
#include "pool.h"
#include "obj.h"
#include "list.h"
#include "inst.h"
#include "req.h"
#include "fcspec.h"
#include "types.h"
#include "fifo.h"

#define REQ_BUFFER_SIZE (4096)
#define REQ_CONTENT_SIZE (REQ_BUFFER_SIZE - FCGI_HEADER_LEN)

typedef struct req_packet {
	req_t *req;
	union {
		FCGI_Header header;
		char buffer[FCGI_HEADER_LEN];
	};
} req_packet_t;

static obj_type_t pool_type;
static obj_type_t inst_handle_type;

static void pool_uninit(pool_t *pool)
{
	assert(list_is_empty(&pool->req_list));
	assert(pool->req_cnt == 0);
	assert(list_is_empty(&pool->inst_list));
	assert(pool->run_cnt == 0);
	free(pool->cmd_line);
	CloseHandle(pool->w32_job);
}

static void inst_handle_uninit(inst_handle_t *hi)
{
	assert(hi->i->state != INST_POOLING);

	if (hi->i->state == INST_RUNNING) {
		req_abort(hi->i->req);
		obj_release(hi->i->req);
		assert(hi->i->remain > 0);
		hi->i->state = INST_IDLE;
		if (!hi->reuse || --hi->i->remain <= 0) {
			inst_abort(hi->i);
		}
	}

	if (hi->i->state == INST_IDLE) {
		assert(hi->i->remain > 0);
		if (timer_start(hi->i->timer)) {
			inst_abort(hi->i);
		} else {
			hi->i->state = INST_POOLING;
			obj_add_ref(hi->i);
			list_add_head(&hi->i->pool->inst_list, &hi->i->pool_entry);
		}
	}

	pool_marry(hi->i->pool);
	obj_release(hi->i);
}

int pool_startup(void)
{
	pool_type.size = sizeof(pool_t);
	pool_type.uninit = (uninit_method_t *)&pool_uninit;
	inst_handle_type.size = sizeof(inst_handle_t);
	inst_handle_type.uninit = (uninit_method_t *)&inst_handle_uninit;
	return 0;
}

static HANDLE create_job(void)
{
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit;
	HANDLE job = CreateJobObject(NULL, NULL);
	if (job == NULL)
		return NULL;

	if (!QueryInformationJobObject(job, JobObjectExtendedLimitInformation,
		&limit, sizeof(limit), NULL)) {
		CloseHandle(job);
		return NULL;
	}
	limit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation,
		&limit, sizeof(limit))) {
		CloseHandle(job);
		return NULL;
	}
	return job;
}

pool_t *pool_create(const char *cmd_line, int queue_length,
	int max_instances, int idle_time, int max_requests)
{
	pool_t *pool = (pool_t *)obj_alloc(&pool_type);
	if (pool == NULL)
		return NULL;

	// pool object initialization begins
	pool->cmd_line = _strdup(cmd_line);
	if (pool->cmd_line == NULL) {
		obj_free(pool);
		return NULL;
	}

	pool->w32_job = create_job();
	if (pool->w32_job == NULL) {
		free(pool->cmd_line);
		obj_free(pool);
		return NULL;
	}

	pool->queue_length = queue_length;
	pool->max_instances = max_instances;
	pool->idle_time = idle_time;
	pool->max_requests = max_requests;
	list_init(&pool->req_list);
	pool->req_cnt = 0;
	list_init(&pool->inst_list);
	pool->run_cnt = 0;
	// pool object initialization ends

	return pool;
}

inst_handle_t *pool_acquire_inst(pool_t *pool)
{
	inst_handle_t *hi;
	int e;

	hi = (inst_handle_t *)obj_alloc(&inst_handle_type);
	if (hi == NULL)
		return NULL;

	if (list_is_empty(&pool->inst_list)) {
		if (pool->max_instances > 0 && pool->run_cnt >= pool->max_instances)
			return NULL;
		hi->i = inst_create(pool);
		assert(hi->i->state == INST_IDLE);
	} else {
		hi->i = container_of(pool->inst_list.next, inst_t, pool_entry);
		assert(hi->i->state = INST_POOLING);
		e = timer_stop(hi->i->timer);
		assert(e == 0);
		// reference to inst object transfers from pool (by list entry) to user
		// no reference count modification is required
		list_remove(&hi->i->pool_entry);
		hi->i->state = INST_IDLE;
	}
	hi->reuse = 0;
	return hi;
}

static void transfer_write_cb(void *u, ssize_t size)
{
	req_packet_t *rp = (req_packet_t *)u;
	if (size < 0) {
		req_abort(rp->req);
	}
	obj_release(rp->req);
	free(rp);
}


static int transfer_req_eof(req_t *r, unsigned char type)
{
	fifo_t *f;
	req_packet_t *rp = (req_packet_t *)malloc(sizeof(req_packet_t));
	if (rp == NULL)
		return -1;
	rp->req = r; // add reference later
	memset(&rp->header, 0, sizeof(rp->header));
	rp->header.version = FCGI_VERSION_1;
	assert(type == FCGI_PARAMS || type == FCGI_STDIN);
	rp->header.type = type;
	if (type == FCGI_PARAMS) {
		f = &r->f_params;
	} else {
		f = &r->f_stdin;
	}
	if (inst_write(rp->req->hi->i, rp->buffer, FCGI_HEADER_LEN, &transfer_write_cb, rp)) {
		req_abort(rp->req);
		free(rp);
		return -1;
	}
	obj_add_ref(r);
	return 0;
}

static int transfer_req(req_t *r, unsigned char type);

static void transfer_read_cb(void *u, ssize_t size)
{
	req_packet_t *rp = (req_packet_t *)u;
	size_t padding;
	if (size < 0)
		goto err;
	if (rp->req->state != REQ_ACTIVE)
		goto err;
	rp->header.contentLengthB1 = (unsigned char)((size_t)size >> 8);
	rp->header.contentLengthB0 = (unsigned char)(size_t)size;
	padding = 7 - ((size + 7) & 7);
	rp->header.paddingLength = padding;
	if (inst_write(rp->req->hi->i, rp->buffer, FCGI_HEADER_LEN + size + padding, &transfer_write_cb, rp))
		goto err;
	// rp belongs to transfer_write_cb()
	assert(size <= REQ_CONTENT_SIZE);
	if (size < REQ_CONTENT_SIZE) {
		// end of file
		if (transfer_req_eof(rp->req, rp->header.type))
			req_abort(rp->req);
	} else {
		if (transfer_req(rp->req, rp->header.type))
			req_abort(rp->req);
	}
	return;
err:
	req_abort(rp->req);
	obj_release(rp->req);
	free(rp);
}

static int transfer_req(req_t *r, unsigned char type)
{
	fifo_t *f;
	req_packet_t *rp = (req_packet_t *)malloc(sizeof(req_packet_t) + REQ_CONTENT_SIZE);
	if (rp == NULL)
		return -1;
	rp->req = r; // add reference later
	memset(&rp->header, 0, sizeof(rp->header));
	rp->header.version = FCGI_VERSION_1;
	assert(type == FCGI_PARAMS || type == FCGI_STDIN);
	rp->header.type = type;
	if (type == FCGI_PARAMS) {
		f = &r->f_params;
	} else {
		f = &r->f_stdin;
	}
	if (fifo_read(f, &rp->buffer[FCGI_HEADER_LEN], REQ_CONTENT_SIZE, &transfer_read_cb, rp)) {
		free(rp);
		return -1;
	}
	obj_add_ref(r);
	return 0;
}

static void marry_cb(void *u, ssize_t size)
{
	req_t *req = (req_t *)u;
	if (req->state == REQ_BEGIN && size == sizeof(FCGI_BeginRequestRecord)) {
		req->state = REQ_ACTIVE;
		(*req->begin_cb)(req->begin_u, 0);
		if (transfer_req(req, FCGI_PARAMS)) {
			req_abort(req);
		}
		if (transfer_req(req, FCGI_STDIN)) {
			req_abort(req);
		}
	} else {
		// failed to send begin record
		req_abort(req);
		(*req->begin_cb)(req->begin_u, -1);
	}
	obj_release(req);
}

void pool_marry(pool_t *pool)
{
	req_t *req;
	inst_handle_t *hi;
	static const FCGI_BeginRequestRecord rec = {
		{ FCGI_VERSION_1, FCGI_BEGIN_REQUEST, 0, 0, 0, 8, },
		{ 0, FCGI_RESPONDER, FCGI_KEEP_CONN, },
	};

	while (pool->req_cnt != 0 &&
		(!list_is_empty(&pool->inst_list)
			|| pool->max_instances <= 0
			|| pool->run_cnt < pool->max_instances))
	{
		req = container_of(pool->req_list.next, req_t, rq_entry);
		assert(req->state == REQ_PENDING);
		assert(req->pool == pool);
		obj_release(pool); // for req->pool
		list_remove(&req->rq_entry);
		--pool->req_cnt;
		// reference to req transfers from pool (by list entry) to scope
		hi = pool_acquire_inst(pool);
		if (hi == NULL) {
			// failed to acquire instance
			req->state = REQ_END;
			(*req->begin_cb)(req->begin_u, -1);
			obj_release(req);
			continue;
		}
		assert(hi->i->state == INST_IDLE);
		// switch req state to REQ_BEGIN
		req->hi = hi;
		req->state = REQ_BEGIN;
		// switch inst state to INST_RUNNING
		hi->i->state = INST_RUNNING;
		hi->i->req = (req_t *)obj_add_ref(req);
		// send begin record
		if (inst_write(hi->i, (const char *)&rec, sizeof(rec), &marry_cb, req)) {
			// failed to send begin record
			req_abort(req);
			(*req->begin_cb)(req->begin_u, -1);
			obj_release(req);
			continue;
		}
		// reference to req transfers to marry_cb
	}
}
