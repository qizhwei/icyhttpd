#include "obp.h"

ObObjectType ObpDirectoryObjectType;
ObpDirectoryObject *ObpRootDirectory;

int ObInitializeSystem(void)
{
	ObInitializeObjectType(&ObpDirectoryObjectType, NULL, &ObpDirectoryParse, &ObpDirectoryInsert);
	ObpRootDirectory = ObCreateDirectoryObject(NULL, NULL);
	if (ObpRootDirectory == NULL) {
		return 1;
	}
	
	return 0;
}
