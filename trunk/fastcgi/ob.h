#ifndef _OB_H
#define _OB_H

#include <stddef.h>

typedef void ObCloseRoutine(void *object);
typedef void * ObParseRoutine(void *object, const char *name);
typedef int ObInsertRoutine(void *object, const char *name, void *target);

typedef struct _ObObjectType {
	struct _ObObjectType *ParentType;
	ObCloseRoutine *Close;
	ObParseRoutine *Parse;
	ObInsertRoutine *Insert;
} ObObjectType;

typedef struct _ObDirectoryObject ObDirectoryObject;

extern int ObInitializeSystem(void);
extern void ObInitializeObjectType(ObObjectType *type, ObObjectType *parentType, ObCloseRoutine *close, ObParseRoutine *parse, ObInsertRoutine *insert);
extern void * ObCreateObject(ObObjectType *type, size_t size, void *root, const char *path);
extern void * ObReferenceObjectByPointer(void *object, ObObjectType *type);
extern void * ObReferenceObjectByName(void *root, const char *path, ObObjectType *type);
extern void ObDereferenceObject(void *object);
extern void * ObParseObject(void *object, const char *name);
extern int ObInsertObject(void *object, const char *name, void *target);
extern ObDirectoryObject * ObCreateDirectoryObject(void *root, const char *path);
extern int ObGetReferenceCount(void *object);

#endif
