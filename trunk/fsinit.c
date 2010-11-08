#include "fsp.h"

ObObjectType FsVirtualDirectoryType;
ObObjectType FsFileObjectType;
ObDirectoryObject *FspFsDirectory;

int FsInitializeSystem(void)
{
	ObInitializeObjectType(&FsVirtualDirectoryType, &FsFileObjectType,
		NULL, &FspVirtualDirectoryParse, &FspVirtualDirectoryInsert);
	ObInitializeObjectType(&FsFileObjectType, NULL,
		&FsFileObjectClose, &FsFileObjectParse, NULL);
		
	FspFsDirectory = ObCreateDirectoryObject(NULL, "\\FileSystem");
	if (FspFsDirectory == NULL) {
		return 1;
	}
	
	return 0;
}
