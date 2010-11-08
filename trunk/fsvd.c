#include "fsp.h"

FsVirtualDirectory * FsCreateVirtualDirectory(const char *name, const char *fileName)
{
	FsVirtualDirectory *vd;
	
	vd = ObCreateObject(&FsVirtualDirectoryType, sizeof(FsVirtualDirectory), FspFsDirectory, name);
	if (vd == NULL) {
		return NULL;
	}
	
	if (FspInitializeFileObject(&vd->FileObject, fileName)) {
		ObDereferenceObject(vd);
		return NULL;
	}
	
	vd->NameMap = RtlCreateStringMap(1);
	if (vd->NameMap == NULL) {
		ObDereferenceObject(vd);
	}
	
	return vd;
}

void * FspVirtualDirectoryParse(void *object, const char *name)
{
	FsVirtualDirectory *vd = object;
	void *value;
	
	// Find the name in the map
	if (RtlQueryMap(&vd->NameMap, (void *)name, &value, 0)) {
		
		// If not found, then use the physical file system
		return FsFileObjectParse(object, name);
	}
	
	return ObReferenceObjectByPointer(value, &FsFileObjectType);
}

int FspVirtualDirectoryInsert(void *object, const char *name, void *target)
{
	FsVirtualDirectory *vd = object;
	char *dupName = RtlDuplicateString(name);
	
	if (dupName == NULL) {
		return 1;
	}
	
	// Add reference to target object
	ObReferenceObjectByPointer(target, NULL);
	
	if (RtlInsertMap(&vd->NameMap, (void *)dupName, target)) {
		ObDereferenceObject(target);
		RtlFreeHeap(dupName);
		return 1;
	}
	
	return 0;	
}
