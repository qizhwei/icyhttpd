#ifndef _FS_H
#define _FS_H

#include "ob.h"

typedef struct _FsFileObject FsFileObject;
typedef struct _FsVirtualDirectory FsVirtualDirectory;

extern ObObjectType FsVirtualDirectoryType;
extern ObObjectType FsFileObjectType;

extern int FsInitializeSystem(void);
extern FsVirtualDirectory * FsCreateVirtualDirectory(const char *name, const char *fileName);

#endif
