#include "obp.h"

ObDirectoryObject * ObCreateDirectoryObject(void *root, const char *path)
{
	ObDirectoryObject *dir = (ObDirectoryObject *)
		ObCreateObject(&ObDirectoryObjectType, sizeof(ObDirectoryObject), root, path);
	if (dir == NULL) {
		return NULL;
	}
	
	dir->NameMap = RtlCreateStringMap(1);
	if (dir->NameMap == NULL) {
		ObDereferenceObject(dir);
		return NULL;
	}
	
	return dir;
}

void * ObpDirectoryParse(void *object, const char *name)
{
	ObDirectoryObject *dir = (ObDirectoryObject *)object;
	void *value;
	
	if (RtlQueryMap(&dir->NameMap, (void *)name, &value, 0)) {
		return NULL;
	}
	
	return ObReferenceObjectByPointer(value, NULL);
}

int ObpDirectoryInsert(void *object, const char *name, void *target)
{
	ObDirectoryObject *dir = (ObDirectoryObject *)object;
	char *dupName = RtlDuplicateString(name);
	
	if (dupName == NULL) {
		return 1;
	}
	
	// Add reference to target object
	ObReferenceObjectByPointer(target, NULL);
	
	if (RtlInsertMap(&dir->NameMap, (void *)dupName, target)) {
		ObDereferenceObject(target);
		RtlFreeHeap(dupName);
		return 1;
	}
	
	return 0;
}
