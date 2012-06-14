#ifndef _FCPOOL_H
#define _FCPOOL_H

// fcpool2 is written by iceboy <iceboy_at_iceboy_dot_org>
//
// types and functions exported here starts with prefix 'fc_'
//
// the following functions MUST be called in the same Windows
// thread:
//   * fc_close()
//   * fc_abort_request()
//   * fc_begin_request()
//   * fc_read_request()
//   * fc_write_request()
//   * fc_write_param_request()
//
// the thread MUST be in alertable wait state while waiting.
// for example, a thread with a following main loop staisfies
// the above requirements:
//
// while (1)
//     SleepEx(INFINITE, TRUE);
//
// there is a main concept called object. an object is created
// by the fc_create_xxx() function, returns a pointer which is
// actually a reference to that object. references can be
// duplicated if needed, this is done by calling fc_duplicate().
// all references should be closed by calling fc_close() when
// no longer needed
//
// there is an other convention for asynchronous callbacks.
// these callbacks are invoked to indicate operation completion
// if and only if the return value of the operation function
// indicates success:
//   * if the operation function succeeded, the callback
//     function is called even on failure or cancellation
//     of the operation
//   * if the operation function failed, the callback function
//     is never called
//
// two parameters are introduced in the operation function:
//     cb - pointer to the callback function
//     u - user context
//
// when callback functions are called, the specified user context
// is used as the first parameter

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// type definitions
typedef int fc_ssize_t;
typedef struct fc_pool fc_pool_t;
typedef struct fc_request fc_request_t;
typedef void fc_begin_request_callback_t(void *u, int err);
typedef void fc_completion_t(void *u, fc_ssize_t size);

// fc_startup() startup the entire system
// this function should be called before using any of the below functions
extern void fc_startup(void);

// fc_close() closes an object
// object - pointer to the object
extern void fc_close(void *object);

// fc_duplicate() duplicates an reference to an object
// object - pointer to the object
// the original pointer is returned which indicates the new reference
//     (and that pointer has to be dereferenced twice)
extern void *fc_duplicate(void *object);

// fc_create_pool() creates a pool object
// cmd_line - the command line to create new instance (null-terminated string)
// queue_length - maximum length of the queue containing pending requests
// max_instances - maximum simutaneously running instances
// idle_time - time (in milliseconds) to pooling an instance before killing it
// max_requests - maximum requests an instance can handle, typically 499 for php-cgi
// returns a reference to the pool object on success, NULL on error
extern fc_pool_t *fc_create_pool(const char *cmd_line, int queue_length,
	int max_instances, int idle_time, int max_requests);

// fc_create_request() creates a request object
// returns a reference to the request object on success, NULL on error
extern fc_request_t *fc_create_request(void);

// fc_abort_request() aborts a request
// the corresponding instance is released and all pending operation is cancelled
// r - the pointer to the request object
extern void fc_abort_request(fc_request_t *r);

// fc_begin_request() begins a request
// beginning a request binds an instance to the request object
// r - the pointer to the request object
// pool - the pointer to the pool object from which the instance is acquired
// cb, u - callback function and user context
// returns 0 on success, -1 on error
extern int fc_begin_request(fc_request_t *r, fc_pool_t *pool,
	fc_begin_request_callback_t *cb, void *u);

// fc_read_request() reads from the standard output stream of the request
// r - the pointer to the request object
// buf, size - pointer and size of a buffer
// cb, u - callback function and user context
// returns 0 on success, -1 on error
extern int fc_read_request(fc_request_t *r, char *buf, size_t size,
	fc_completion_t *cb, void *u);

// fc_write_request() writes to the standard input stream of the request
// r - the pointer to the request object
// buf, size - pointer and size of a buffer
// cb, u - callback function and user context
// returns 0 on success, -1 on error
extern int fc_write_request(fc_request_t *r, const char *buf, size_t size,
	fc_completion_t *cb, void *u);

// fc_write_param_request() writes to the param stream of the request
// r - the pointer to the request object
// buf, size - pointer and size of a buffer
// cb, u - callback function and user context
// returns 0 on success, -1 on error
extern int fc_write_param_request(fc_request_t *r, const char *buf, size_t size,
	fc_completion_t *cb, void *u);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
