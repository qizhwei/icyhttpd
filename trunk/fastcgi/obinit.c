#include "obp.h"

ObObjectType ObDirectoryObjectType;
ObDirectoryObject *ObpRootDirectory;

int ObInitializeSystem(void)
{
	ObInitializeObjectType(&ObDirectoryObjectType, NULL, NULL, &ObpDirectoryParse, &ObpDirectoryInsert);
	ObpRootDirectory = ObCreateDirectoryObject(NULL, NULL);
	if (ObpRootDirectory == NULL) {
		return 1;
	}
	
	return 0;
}
