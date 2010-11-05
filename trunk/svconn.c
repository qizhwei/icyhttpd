#include "svp.h"
#include "rtl.h"

int SvpAcceptConnection(SvpListener *listener)
{
	SOCKET socket;
	SvpConnection *connection;
	
	do {
		socket = accept(listener->Socket, NULL, NULL);
		if (socket != INVALID_SOCKET) {
			
			// Create a connection object
			connection = RtlAllocateHeap(sizeof(SvpConnection));
			if (connection == NULL) {
				closesocket(socket);
				break;
			}
			
			// Initialize the connection object
			connection->ReferenceCount = 1;
			connection->Socket = socket;
			
			
			continue;
		}
	} while (0);
	
	if (WSAGetLastError() != WSAEWOULDBLOCK) {
		return 1;
	}
	
	return 0;
}
