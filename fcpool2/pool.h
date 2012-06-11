#ifndef _POOL_H
#define _POOL_H

typedef struct pool pool_t;

typedef struct inst_handle {
	// reference to inst object, user should always reference by handle
	struct inst *i;
	int reuse;
} inst_handle_t;

typedef void begin_req_callback_t(void *u, struct req *req);

extern void pool_startup(void);
extern pool_t *pool_create(const char *cmd_line, int queue_length,
	int max_instances, int idle_time, int max_requests);
extern inst_handle_t *pool_acquire_inst(pool_t *pool);
extern void pool_marry(pool_t *pool);

#endif
