#ifndef _SocketProvider_H
#define _SocketProvider_H

#include "Types.h"
#include "Win32.h"

namespace Httpd
{
	class SocketProvider: NonCopyable
	{
	public:
		static SocketProvider &Instance();
		SOCKET Create();
		void Destroy(SOCKET s);

	public:
		LPFN_TRANSMITFILE TransmitFile() const { return pfnTransmitFile; }
		LPFN_ACCEPTEX AcceptEx() const { return pfnAcceptEx; }
		LPFN_GETACCEPTEXSOCKADDRS GetAcceptExSockaddrs() const { return pfnGetAcceptExSockaddrs; }
		LPFN_TRANSMITPACKETS TransmitPackets() const { return pfnTransmitPackets; }
		LPFN_CONNECTEX ConnectEx() const { return pfnConnectEx; }
		LPFN_DISCONNECTEX DisconnectEx() const { return pfnDisconnectEx; }
		LPFN_WSARECVMSG WSARecvMsg() const { return pfnWSARecvMsg; }

	private:
		SocketProvider();

		// this class has no destructor, declaration only
		~SocketProvider();

	private:
		LPFN_TRANSMITFILE pfnTransmitFile;
		LPFN_ACCEPTEX pfnAcceptEx;
		LPFN_GETACCEPTEXSOCKADDRS pfnGetAcceptExSockaddrs;
		LPFN_TRANSMITPACKETS pfnTransmitPackets;
		LPFN_CONNECTEX pfnConnectEx;
		LPFN_DISCONNECTEX pfnDisconnectEx;
		LPFN_WSARECVMSG pfnWSARecvMsg;
	};
}

#endif
