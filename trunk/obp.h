#ifndef _OBP_H
#define _OBP_H

#include "ob.h"

typedef struct _ObpObjectHeader {
	int ReferenceCount;
	ObObjectType *ObjectType;
} ObpObjectHeader;

static inline ObpObjectHeader * ObpGetObjectHeader(void *object)
{
	return (ObpObjectHeader *)(((char *)object) - sizeof(ObpObjectHeader));
}

#endif
