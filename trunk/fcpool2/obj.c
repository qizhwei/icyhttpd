#include <stdlib.h>
#include <assert.h>
#include "obj.h"

typedef struct hdr {
	obj_type_t *type;
	int ref_count;
} hdr_t;

#define REF_MAX (32)
#define HDR_TO_BODY(h) ((void *)((char *)(h) + sizeof(hdr_t)))
#define BODY_TO_HDR(b) ((void *)((char *)(b) - sizeof(hdr_t)))

void *obj_alloc(obj_type_t *t)
{
	hdr_t *h = (hdr_t *)malloc(sizeof(hdr_t) + t->size);
	if (h == NULL)
		return NULL;
	h->type = t;
	h->ref_count = 1;
	return HDR_TO_BODY(h);
}

void obj_free(void *o)
{
	free(BODY_TO_HDR(o));
}

void *obj_add_ref(void *o)
{
	hdr_t *h = (hdr_t *)BODY_TO_HDR(o);
	assert(h->ref_count > 0 && h->ref_count < REF_MAX);
	++h->ref_count;
	return o;
}

void obj_release(void *o)
{
	hdr_t *h = (hdr_t *)BODY_TO_HDR(o);
	assert(h->ref_count > 0 && h->ref_count < REF_MAX);
	if (--h->ref_count == 0) {
		(*h->type->uninit)(o);
		obj_free(o);
	}
}
