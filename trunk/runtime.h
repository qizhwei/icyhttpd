#ifndef _RUNTIME_H
#define _RUNTIME_H

#include <sys/types.h>
#include <stddef.h>

#define MAYFAIL(return_code)
#define NOFAIL

typedef void proc_t(void *u);
typedef MAYFAIL(-1) ssize_t io_proc_t(void *u, void *buffer, size_t size);

extern void runtime_abort(char *what);
extern void runtime_ignore(char *what);

#endif
