#ifndef _RUNTIME_H
#define _RUNTIME_H

#include <sys/types.h>
#include <stddef.h>

// MAYFAIL indicates a function may fail with the specified return code
// NOFAIL indicates a function may not fail
// a void function MUST never fail
// otherwise a function may or may not fail depends on implementation
#define MAYFAIL(return_code)
#define NOFAIL

// general purpose procedure prototypes
typedef void proc_t(void *u);
typedef MAYFAIL(-1) ssize_t io_proc_t(void *u, void *buffer, size_t size);

// runtime functions
extern void runtime_abort(char *what);
extern void runtime_ignore(char *what);

#endif
