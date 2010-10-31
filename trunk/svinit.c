#include "svp.h"
#include "win32.h"

ListEntry SvpSiteList;
ListEntry SvpListenerList;
HANDLE SvpAcceptEvent;
static int Initialized = 0;

int SvpInitialize(void)
{
	WSADATA wsaData;
	
	if (!Initialized) {
		
		// Initialize list
		InitializeListHead(&SvpSiteList);
		InitializeListHead(&SvpListenerList);
		
		// Create an event for accept operation
		SvpAcceptEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
		if (SvpAcceptEvent == NULL) {
			return 1;
		}
		
		// Startup winsock
		if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
			return 1;
		}
	
		Initialized = 1;
	}
	return 0;
}
