#include "obp.h"
#include "rtl.h"
#include <assert.h>

void * ObCreateObject(ObObjectType *type, size_t size)
{
	ObpObjectHeader *header = RtlAllocateHeap(sizeof(ObpObjectHeader) + size);
	
	if (header == NULL) {
		return NULL;
	}
	
	header->ReferenceCount = 1;
	header->ObjectType = type;
	
	return &header[1];
}

void * ObReferenceObjectByPointer(void *object, ObObjectType *type)
{
	ObpObjectHeader *header = ObpGetObjectHeader(object);
	
	if (type != NULL && header->ObjectType != type) {
		return NULL;
	}
	
	header->ReferenceCount += 1;
	return object;
}

void ObDereferenceObject(void *object)
{
	ObpObjectHeader *header = ObpGetObjectHeader(object);
	ObObjectType *type;
	
	assert(header->ReferenceCount > 0);
	header->ReferenceCount -= 1;
	if (header->ReferenceCount == 0) {
		type = header->ObjectType;
		type->Close(object);
		RtlFreeHeap(header);
	}
}
