#include "fsp.h"

FsFileObject * FspCreateFileObject(const char *name, const char *fileName)
{
	FsFileObject *file;
	
	file = ObCreateObject(&FsFileObjectType, sizeof(FsFileObject), FspFsDirectory, name);
	if (file == NULL) {
		return NULL;
	}
	
	if (FspInitializeFileObject(file, fileName)) {
		ObDereferenceObject(file);
		return NULL;
	}
	
	return file;
}

int FspInitializeFileObject(FsFileObject *file, const char *fileName)
{
	if (fileName == NULL) {
		file->FileName = NULL;
	} else {
		file->FileName = RtlDuplicateString(fileName);
		if (file->FileName == NULL) {
			return 1;
		}
	}
	
	return 0;
}

void FsFileObjectClose(void *object)
{
	FsFileObject *file = object;
	
	if (file->FileName != NULL) {
		RtlFreeHeap(file->FileName);
	}
}

void * FsFileObjectParse(void *object, const char *name)
{
	FsFileObject *file = object;
	const char *p;
	int valid = 0, invalid = 0;
	char *newName;
	FsFileObject *result;
	
	// Validate the name
	for (p = name; *p; ++p) {
		switch (*p) {
			case ' ':
			case '.':
				break;
			case '\\':
			case '/':
			case ':':
			case '*':
			case '?':
			case '\"':
			case '<':
			case '>':
			case '|':
			case '\'':
				invalid = 1;
				break;
			default:
				valid = 1;
				break;
		}
	}
	
	if (!valid || invalid) {
		return NULL;
	}
	
	newName = RtlDuplicateString(file->FileName);
	if (newName == NULL) {
		return NULL;
	}
	
	if (RtlConcatString(&newName, "\\")) {
		RtlFreeHeap(newName);
		return NULL;
	}

	if (RtlConcatString(&newName, name)) {
		RtlFreeHeap(newName);
		return NULL;
	}
	
	result = FspCreateFileObject(NULL, newName);
	RtlFreeHeap(newName);
	
	if (result == NULL) {
		return NULL;
	}
	
	return result;
}
