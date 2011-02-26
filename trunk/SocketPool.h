#ifndef _SOCKETPOOL_H
#define _SOCKETPOOL_H

#include "Types.h"
#include "Win32.h"

namespace Httpd
{
	class SocketPool: NonCopyable
	{
		friend class Socket;
	public:
		static SocketPool &Instance();
		SOCKET Pop();
		void Push(SOCKET s, bool canReuse);

	private:
		SocketPool();

		// this class has no destructor, declaration only
		~SocketPool();

	private:
		LPFN_TRANSMITFILE TransmitFile;
		LPFN_ACCEPTEX AcceptEx;
		LPFN_GETACCEPTEXSOCKADDRS GetAcceptExSockaddrs;
		LPFN_TRANSMITPACKETS TransmitPackets;
		LPFN_CONNECTEX ConnectEx;
		LPFN_DISCONNECTEX DisconnectEx;
		LPFN_WSARECVMSG WSARecvMsg;
	};
}

#endif
