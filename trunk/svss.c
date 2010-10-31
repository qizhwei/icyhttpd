#include "svp.h"
#include <assert.h>

int SvpAcceptSession(SvpListener *listener)
{
	SOCKET socket;
	
	do {
		socket = accept(listener->Socket, NULL, NULL);
		if (socket != INVALID_SOCKET) {
			
			// TODO: Create session!
			assert(0);
			
			continue;
		}
	} while (0);
	
	if (WSAGetLastError() != WSAEWOULDBLOCK) {
		return 1;
	}
	
	return 0;
}