#include "obp.h"
#include "rtl.h"
#include <string.h>
#include <assert.h>

void * ObCreateObject(ObObjectType *type, size_t size, void *root, const char *path)
{
	ObpObjectHeader *header = RtlAllocateHeap(sizeof(ObpObjectHeader) + size);
	char *curPath, *dupPath, *nextPath;
	void *curRoot, *nextRoot;
	
	if (header == NULL) {
		return NULL;
	}
	
	header->ReferenceCount = 1;
	header->ObjectType = type;
	
	if (path == NULL) {
		header->Parent = NULL;
	} else {
		curPath = dupPath = RtlDuplicateString(path);
		
		if (root == NULL || path[0] == '\\') {
			curRoot = ObpRootDirectory;
		} else {
			curRoot = root;
		}
		
		ObReferenceObjectByPointer(curRoot, NULL);
		
		if (curPath[0] == '\\') {
			curPath += 1;
		}
		
		curPath = strtok(curPath, "\\");
		if (curPath == NULL) {
			RtlFreeHeap(header);
			RtlFreeHeap(dupPath);
			return NULL;
		}
		
		while (1) {
			nextPath = strtok(NULL, "\\");
			if (nextPath == NULL) {
				if (ObInsertObject(curRoot, curPath, &header[1])) {
					RtlFreeHeap(header);
					RtlFreeHeap(dupPath);
					return NULL;
				}
				assert(header->ReferenceCount == 2);
				header->Parent = curRoot;
				break;
			} else {
				nextRoot = ObParseObject(curRoot, curPath);
				ObDereferenceObject(curRoot);
				if (nextRoot == NULL) {
					RtlFreeHeap(header);
					RtlFreeHeap(dupPath);
					return NULL;
				}
				curPath = nextPath;
				curRoot = nextRoot;
			}
		}
		
		RtlFreeHeap(dupPath);
	}
	
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

void * ObReferenceObjectByName(void *root, const char *path, ObObjectType *type)
{
	char *curPath, *dupPath;
	void *curRoot, *nextRoot;
	
	curPath = dupPath = RtlDuplicateString(path);
	
	if (root == NULL || path[0] == '\\') {
		curRoot = ObpRootDirectory;
	} else {
		curRoot = root;
	}
	
	ObReferenceObjectByPointer(curRoot, NULL);
	
	if (curPath[0] == '\\') {
		curPath += 1;
	}
	
	curPath = strtok(curPath, "\\");
	if (curPath == NULL) {
		RtlFreeHeap(dupPath);
		return NULL;
	}
	
	do {
		nextRoot = ObParseObject(curRoot, curPath);
		ObDereferenceObject(curRoot);
		if (nextRoot == NULL) {
			RtlFreeHeap(dupPath);
			return NULL;
		}
		curRoot = nextRoot;
		curPath = strtok(NULL, "\\");
	} while (curPath != NULL);
	
	RtlFreeHeap(dupPath);
	return curRoot;
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
		if (header->Parent != NULL) {
			ObDereferenceObject(header->Parent);
		}
		RtlFreeHeap(header);
	}
}

void * ObParseObject(void *object, const char *name)
{
	ObpObjectHeader *header = ObpGetObjectHeader(object);
	ObObjectType *type = header->ObjectType;
	
	if (type->Parse == NULL) {
		return NULL;
	}
	
	return type->Parse(object, name);
}

int ObInsertObject(void *object, const char *name, void *target)
{
	ObpObjectHeader *header = ObpGetObjectHeader(object);
	ObObjectType *type = header->ObjectType;
	
	if (type->Insert == NULL) {
		return 1;
	}
	
	return type->Insert(object, name, target);
}

int ObGetReferenceCount(void *object)
{
	ObpObjectHeader *header = ObpGetObjectHeader(object);
	return header->ReferenceCount;
}
