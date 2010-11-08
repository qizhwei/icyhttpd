#ifndef _OBP_H
#define _OBP_H

#include "ob.h"
#include "rtl.h"

typedef struct _ObpObjectHeader {
	int ReferenceCount;
	ObObjectType *ObjectType;
	void *Parent;
} ObpObjectHeader;

static inline ObpObjectHeader * ObpGetObjectHeader(void *object)
{
	return (ObpObjectHeader *)(((char *)object) - sizeof(ObpObjectHeader));
}

struct _ObDirectoryObject {
	RtlMap *NameMap;
};

extern ObObjectType ObDirectoryObjectType;
extern ObDirectoryObject *ObpRootDirectory;
extern void * ObpDirectoryParse(void *object, const char *name);
extern int ObpDirectoryInsert(void *object, const char *name, void *target);

#endif
