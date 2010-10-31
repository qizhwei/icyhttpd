#include "fcp.h"

HANDLE FcpJobObject;
static int Initialized;

int FcpInitialize(void)
{
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit;

	if (!Initialized) {
		
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
		
		Initialized = 1;
	}

	return 0;
}
