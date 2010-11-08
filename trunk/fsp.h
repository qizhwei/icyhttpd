#ifndef _FSP_H
#define _FSP_H

#include "fs.h"
#include "rtl.h"

extern ObDirectoryObject *FspFsDirectory;

struct _FsFileObject {
	char *FileName;
	// TODO: Handler?
};

struct _FsVirtualDirectory {
	FsFileObject FileObject;
	RtlMap *NameMap;
};

extern FsFileObject * FspCreateFileObject(const char *name, const char *fileName);
extern int FspInitializeFileObject(FsFileObject *file, const char *fileName);
extern void FsFileObjectClose(void *object);
extern void * FsFileObjectParse(void *object, const char *name);
extern void FspVirtualDirectoryClose(void *object);
extern void * FspVirtualDirectoryParse(void *object, const char *name);
extern int FspVirtualDirectoryInsert(void *object, const char *name, void *target);

#endif