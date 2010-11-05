#ifndef _OB_H
#define _OB_H

#include <stddef.h>

typedef void ObCloseRoutine(void *object);
typedef void * ObParseRoutine(void *object, char *relativePath);

typedef struct _ObObjectType {
	ObCloseRoutine *Close;
	ObParseRoutine *Parse;
} ObObjectType;

extern void ObInitializeObjectType(ObObjectType *type, ObCloseRoutine *closeRoutine, ObParseRoutine *parseRoutine);
extern void * ObCreateObject(ObObjectType *type, size_t size);
extern void * ObReferenceObjectByPointer(void *object, ObObjectType *type);
extern void ObDereferenceObject(void *object);

#endif
