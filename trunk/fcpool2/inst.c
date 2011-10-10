#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <windows.h>
#include "fcspec.h"
#include "internal.h"
#include "inst.h"
#include "obj.h"
#include "timer.h"
#include "fifo.h"

#define PIPE_BUFFER_SIZE (4096)

typedef struct in_packet {
	OVERLAPPED w32_overlap;
	union {
		inst_t *inst; // valid when reading inst
		req_t *req; // valid when passing data to req
	};
	size_t size;
	union {
		FCGI_Header header;
		char buffer[FCGI_HEADER_LEN];
	};
} in_packet_t;

typedef struct out_packet {
	OVERLAPPED w32_overlap;
	inst_t *inst;
	completion_t *cb;
	void *u;
} out_packet_t;

static obj_type_t inst_type;

static void inst_uninit(inst_t *i)
{
	assert(i->state == INST_RUNDOWN);
	obj_release(i->pool);
	obj_release(i->timer);
	CloseHandle(i->w32_pipe);
	CloseHandle(i->w32_process);
}

void inst_startup(void)
{
	inst_type.size = sizeof(inst_t);
	inst_type.uninit = (uninit_method_t *)&inst_uninit;
}

static int create_pipe(HANDLE pipe[2])
{
	static unsigned long cnt = 0;
	static const char fmt[] = "\\\\.\\pipe\\fcpool\\critter.%08lx.%08lx";
	char name[48];
	SECURITY_ATTRIBUTES sa;
	
	sprintf(name, fmt, (unsigned long)GetCurrentProcessId(), cnt++);
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	// create server end
	pipe[0] = CreateNamedPipeA(name, FILE_FLAG_OVERLAPPED | PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_READMODE_BYTE, PIPE_UNLIMITED_INSTANCES,
		PIPE_BUFFER_SIZE, PIPE_BUFFER_SIZE, 0, &sa);
	if (pipe[0] == INVALID_HANDLE_VALUE)
		return -1;

	// open client end
	pipe[1] = CreateFileA(name, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (pipe[1] == INVALID_HANDLE_VALUE) {
		CloseHandle(pipe[0]);
		return -1;
	}

	return 0;
}

static HANDLE create_process(const char *cmd_line, HANDLE pipe, HANDLE job)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = pipe;
	si.hStdOutput = INVALID_HANDLE_VALUE;
	si.hStdError = INVALID_HANDLE_VALUE;

	// CreateProcessA() doesn't modify cmd_line (while CreateProcessW() does)
	if (!CreateProcessA(NULL, (LPSTR)cmd_line, NULL, NULL, TRUE,
		CREATE_BREAKAWAY_FROM_JOB | CREATE_SUSPENDED, NULL, NULL, &si, &pi))
		return NULL;

	if (!AssignProcessToJobObject(job, pi.hProcess)
		|| !ResumeThread(pi.hThread)) {
		TerminateProcess(pi.hProcess, 1);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return NULL;
	}

	CloseHandle(pi.hThread);
	return pi.hProcess;
}

static void inst_timer_callback(void *state)
{
	inst_t *i = (inst_t *)state;
	assert(i->state == INST_POOLING);
	list_remove(&i->pool_entry);
	// reference to inst object transfers from pool to scope
	i->state = INST_IDLE;
	inst_abort(i);
	obj_release(i);
}

// d has transfer semantic even if fail
static void write_stdout_cb(void *u, ssize_t s)
{
	in_packet_t *d = (in_packet_t *)u;
	obj_release(d->req);
	free(d);
}

static void write_eof_cb(void *u, ssize_t s)
{
	req_abort((req_t *)u);
}

// d has transfer semantic even if fail
static int on_packet(in_packet_t *d, size_t len)
{
	inst_t *i = d->inst;
	req_t *req;

	switch (d->header.type) {
	case FCGI_STDOUT:
		if (i->state != INST_RUNNING)
			goto err;
		req = i->req;
		assert(req->state == REQ_ACTIVE);
		assert(req->hi->i == i);

		if (fifo_write(&req->f_stdout, &d->buffer[FCGI_HEADER_LEN], len,
			&write_stdout_cb, d))
			goto err;
		// switch semantic of in_packet_t
		d->req = (req_t *)obj_add_ref(req);
		obj_release(i);
		return 0;
	case FCGI_END_REQUEST:
		if (i->state != INST_RUNNING)
			goto err;
		req = i->req;
		assert(req->state == REQ_ACTIVE);
		assert(req->hi->i == i);
		req->hi->reuse = 1;
		free(d);
		obj_release(i);

		// gracefully close stdout stream
		if (fifo_write(&req->f_stdout, NULL, 0, &write_eof_cb, req)) {
			// unlink inst and req
			req_abort(req);
			return -1;
		}
		return 0;
	default:
		// undefined packet type
		free(d);
		obj_release(i);
		return 0;
	}
err:
	obj_release(i);
	free(d);
	return -1;
}

static int read_inst(inst_t *i);

static void CALLBACK read_inst_cb(DWORD err_code, DWORD transferred, LPOVERLAPPED overlapped)
{
	in_packet_t *d = container_of(overlapped, in_packet_t, w32_overlap);
	in_packet_t *d_new;
	inst_t *i = d->inst;
	size_t len, padded_size;
	
	if (err_code != 0)
		goto err;

	d->size += transferred;
	if (d->size >= FCGI_HEADER_LEN) {
		len = ((size_t)d->header.contentLengthB1 << 8) | (size_t)d->header.contentLengthB0;
		padded_size = len + d->header.paddingLength;
	} else {
		padded_size = 0; // unknown
	}

	assert(d->size <= FCGI_HEADER_LEN + padded_size);
	if (d->size >= FCGI_HEADER_LEN + padded_size) {
		// packet completed
		obj_add_ref(i);
		// d has transfer semantic even if fail
		if (on_packet(d, len)) {
			d = NULL;
			goto err;
		}
		read_inst(i);
		obj_release(i);
	} else {
		// read more
		d_new = (in_packet_t *)realloc(d, sizeof(in_packet_t) * padded_size);
		if (d_new == NULL)
			goto err;
		d = d_new;
		memset(&d->w32_overlap, 0, sizeof(d->w32_overlap));
		if (!ReadFileEx(i->w32_pipe, &d->buffer[d->size],
			FCGI_HEADER_LEN + padded_size - d->size, &d->w32_overlap, &read_inst_cb))
			goto err;
	}
	return;
err:
	inst_abort(i);
	obj_release(i); // for d->inst
	free(d);
}

static int read_inst(inst_t *i)
{
	in_packet_t *d = (in_packet_t *)malloc(sizeof(in_packet_t));
	if (d == NULL)
		return -1;
	memset(&d->w32_overlap, 0, sizeof(d->w32_overlap));
	d->inst = i; // add reference later
	d->size = 0;

	if (!ReadFileEx(i->w32_pipe, d->buffer, FCGI_HEADER_LEN,
		&d->w32_overlap, &read_inst_cb)) {
		free(d);
		return -1;
	}

	obj_add_ref(i);
	return 0;
}

inst_t *inst_create(pool_t *pool)
{
	HANDLE pipe[2];
	inst_t *i = (inst_t *)obj_alloc(&inst_type);
	if (i == NULL)
		return NULL;

	// inst object initialization begins
	i->timer = timer_create(pool->idle_time, &inst_timer_callback, i);
	if (i->timer == NULL) {
		obj_free(i);
		return NULL;
	}

	if (create_pipe(pipe)) {
		obj_release(i->timer);
		obj_free(i);
		return NULL;
	}
	i->w32_pipe = pipe[1];
	i->w32_process = create_process(pool->cmd_line, pipe[0], pool->w32_job);

	// close remote pipe handle anyway
	CloseHandle(pipe[0]);

	if (i->w32_process == NULL) {
		CloseHandle(i->w32_pipe);
		obj_release(i->timer);
		obj_free(i);
		return NULL;
	}

	i->state = INST_IDLE;
	i->pool = (pool_t *)obj_add_ref(pool);
	i->remain = pool->max_requests;
	// inst object initialization ends

	++pool->run_cnt;

	// start reading from the application
	if (read_inst(i)) {
		obj_release(i);
		return NULL;
	}

	return i;
}

void inst_abort(inst_t *i)
{
	int e;
	int state = i->state;
	req_t *req;

	if (state == INST_RUNDOWN)
		return;
	i->state = INST_RUNDOWN;
	--i->pool->run_cnt;

	// add reference for dereference
	obj_add_ref(i);

	switch (state) {
	case INST_IDLE:
		break;
	case INST_RUNNING:
		assert(i->req->state == REQ_BEGIN || i->req->state == REQ_ACTIVE);
		req = i->req; // save req since req_abort() changes inst state
		req_abort(req);
		obj_release(req);
		break;
	case INST_POOLING:
		e = timer_stop(i->timer);
		assert(e == 0);
		list_remove(&i->pool_entry);
		obj_release(i);
		break;
	}

	TerminateProcess(i->w32_process, 0); // best effort
	obj_release(i);
}

static void CALLBACK write_inst_cb(DWORD err_code, DWORD transferred, LPOVERLAPPED overlapped)
{
	out_packet_t *o = container_of(overlapped, out_packet_t, w32_overlap);
	if (err_code != 0) {
		(*o->cb)(o->u, -1);
	} else {
		(*o->cb)(o->u, transferred);
	}
	obj_release(o->inst);
	free(o);
}

int inst_write(inst_t *i, const char *buf, size_t size, completion_t *cb, void *u)
{
	out_packet_t *o = (out_packet_t *)malloc(sizeof(out_packet_t));
	if (o == NULL)
		return -1;
	memset(&o->w32_overlap, 0, sizeof(o->w32_overlap));
	o->inst = i; // add reference later
	o->cb = cb;
	o->u = u;
	if (!WriteFileEx(i->w32_pipe, buf, size, &o->w32_overlap, &write_inst_cb)) {
		free(o);
		return -1;
	}

	obj_add_ref(i);
	return 0;
}
