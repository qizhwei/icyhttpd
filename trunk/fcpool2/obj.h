#ifndef _OBJ_H
#define _OBJ_H

#include <stddef.h>

typedef void uninit_method_t(void *u);

typedef struct obj_type {
	size_t size;
	uninit_method_t *uninit;
} obj_type_t;

// obj_alloc() returns an uninitialized object of type t
// the object should either be initialized by user or freed
// by calling obj_free()
extern void *obj_alloc(obj_type_t *t);

// obj_free() frees an uninitialized object
extern void obj_free(void *o);

// obj_add_ref() references an initialized object by incrementing
// the reference count
extern void *obj_add_ref(void *o);

// obj_release() releases an initialized object by decrementing
// the reference count, if the last reference is released,
// the uninitialize routine is called and the object is freed
extern void obj_release(void *o);

#endif
