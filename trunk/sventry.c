#include "svp.h"
#include <assert.h>

int SvThreadEntry(void)
{
	DWORD waitResult;
	
	if (SvpInitialize()) {
		return 1;
	}
	
	if (SvpCreateListeners()) {
		return 1;
	}
	
	if (SvpStartListeners()) {
		return 1;
	}
	
	while (1) {
		waitResult = WaitForSingleObjectEx(SvpAcceptEvent, INFINITE, TRUE);
		
		if (waitResult == WAIT_OBJECT_0) {
			SvpAcceptListeners();
		} else if (waitResult != WAIT_IO_COMPLETION) {
			assert(0);
		}
		
		SleepEx(INFINITE, TRUE);
	}
}
