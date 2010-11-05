#include "fcp.h"
#include "ob.h"

HANDLE FcpJobObject;
ObObjectType FcpPoolObjectType;
ObObjectType FcpProcessObjectType;
ObObjectType FcpRequestObjectType;
ObObjectType FcpWaitBlockObjectType;

int FcInitializeSystem(void)
{
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit;
	
	// Initialize object types
	ObInitializeObjectType(&FcpPoolObjectType, &FcpClosePool, NULL);
	ObInitializeObjectType(&FcpProcessObjectType, &FcpCloseProcess, NULL);
	ObInitializeObjectType(&FcpRequestObjectType, &FcpCloseRequest, NULL);
	ObInitializeObjectType(&FcpWaitBlockObjectType, &FcpCloseWaitBlock, NULL);
	
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
	
	return 0;
}
