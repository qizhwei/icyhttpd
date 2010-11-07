#include "fcp.h"
#include "ob.h"

HANDLE FcpJobObject;
ObObjectType FcpPoolObjectType;
ObObjectType FcpProcessObjectType;
ObObjectType FcpRequestObjectType;
ObObjectType FcpWaitBlockObjectType;
void * FcpPoolDirectory;

int FcInitializeSystem(void)
{
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit;
	
	// Initialize object types
	ObInitializeObjectType(&FcpPoolObjectType, NULL, NULL, NULL);
	ObInitializeObjectType(&FcpProcessObjectType, &FcpCloseProcess, NULL, NULL);
	ObInitializeObjectType(&FcpRequestObjectType, &FcpCloseRequest, NULL, NULL);
	ObInitializeObjectType(&FcpWaitBlockObjectType, &FcpCloseWaitBlock, NULL, NULL);
	
	// create a job object
	FcpJobObject = CreateJobObject(NULL, NULL);
	if (FcpJobObject == NULL) {
		return 1;
	}

	// let all processes assigned to this job object
	// being killed when the job object closed
	if (!QueryInformationJobObject(FcpJobObject, JobObjectExtendedLimitInformation, &limit, sizeof(limit), NULL)) {
		CloseHandle(FcpJobObject);
		return 1;
	}

	limit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

	if (!SetInformationJobObject(FcpJobObject, JobObjectExtendedLimitInformation, &limit, sizeof(limit))) {
		CloseHandle(FcpJobObject);
		return 1;
	}
	
	FcpPoolDirectory = ObCreateDirectoryObject(NULL, "\\FastCgiPool");
	if (FcpPoolDirectory == NULL) {
		CloseHandle(FcpJobObject);
		return 1;
	}
	
	return 0;
}
