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
		SOCKET Create(int af, int type, int protocol);
		void Destroy(SOCKET s);

	public:
		// Redefine these types since MinGW32 doesn't provide this
		typedef
		BOOL
		(PASCAL FAR * PfnTransmitFile)(
			SOCKET hSocket,
			HANDLE hFile,
			DWORD nNumberOfBytesToWrite,
			DWORD nNumberOfBytesPerSend,
			LPOVERLAPPED lpOverlapped,
			LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
			DWORD dwReserved
			);

		typedef
		BOOL
		(PASCAL FAR * PfnAcceptEx)(
			SOCKET sListenSocket,
			SOCKET sAcceptSocket,
			PVOID lpOutputBuffer,
			DWORD dwReceiveDataLength,
			DWORD dwLocalAddressLength,
			DWORD dwRemoteAddressLength,
			LPDWORD lpdwBytesReceived,
			LPOVERLAPPED lpOverlapped
			);

		typedef
		VOID
		(PASCAL FAR * PfnGetAcceptExSockaddrs)(
			PVOID lpOutputBuffer,
			DWORD dwReceiveDataLength,
			DWORD dwLocalAddressLength,
			DWORD dwRemoteAddressLength,
			struct sockaddr **LocalSockaddr,
			LPINT LocalSockaddrLength,
			struct sockaddr **RemoteSockaddr,
			LPINT RemoteSockaddrLength
			);

		typedef struct _TRANSMIT_PACKETS_ELEMENT { 
			ULONG dwElFlags; 
			ULONG cLength; 
			union {
				struct {
					LARGE_INTEGER nFileOffset;
					HANDLE        hFile;
				};
				PVOID             pBuffer;
			};
		} TRANSMIT_PACKETS_ELEMENT, *PTRANSMIT_PACKETS_ELEMENT, FAR *LPTRANSMIT_PACKETS_ELEMENT;

		typedef
		BOOL
		(PASCAL FAR * PfnTransmitPackets) (
			SOCKET hSocket,                             
			LPTRANSMIT_PACKETS_ELEMENT lpPacketArray,                               
			DWORD nElementCount,                
			DWORD nSendSize,                
			LPOVERLAPPED lpOverlapped,                  
			DWORD dwFlags                               
			);

		typedef
		BOOL
		(PASCAL FAR * PfnConnectEx) (
			SOCKET s,
			const struct sockaddr FAR *name,
			int namelen,
			PVOID lpSendBuffer,
			DWORD dwSendDataLength,
			LPDWORD lpdwBytesSent,
			LPOVERLAPPED lpOverlapped
			);

		typedef
		BOOL
		(PASCAL FAR * PfnDisconnectEx) (
			SOCKET s,
			LPOVERLAPPED lpOverlapped,
			DWORD  dwFlags,
			DWORD  dwReserved
			);

		typedef
		INT
		(PASCAL FAR * PfnWSARecvMsg) (
			SOCKET s, 
			LPWSAMSG lpMsg, 
			LPDWORD lpdwNumberOfBytesRecvd, 
			LPWSAOVERLAPPED lpOverlapped, 
			LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
			);

	public:
		PfnTransmitFile TransmitFile() const { return pfnTransmitFile; }
		PfnAcceptEx AcceptEx() const { return pfnAcceptEx; }
		PfnGetAcceptExSockaddrs GetAcceptExSockaddrs() const { return pfnGetAcceptExSockaddrs; }
		PfnTransmitPackets TransmitPackets() const { return pfnTransmitPackets; }
		PfnConnectEx ConnectEx() const { return pfnConnectEx; }
		PfnDisconnectEx DisconnectEx() const { return pfnDisconnectEx; }
		PfnWSARecvMsg WSARecvMsg() const { return pfnWSARecvMsg; }

	private:
		SocketProvider();

		// this class has no destructor, declaration only
		~SocketProvider();

	private:
		PfnTransmitFile pfnTransmitFile;
		PfnAcceptEx pfnAcceptEx;
		PfnGetAcceptExSockaddrs pfnGetAcceptExSockaddrs;
		PfnTransmitPackets pfnTransmitPackets;
		PfnConnectEx pfnConnectEx;
		PfnDisconnectEx pfnDisconnectEx;
		PfnWSARecvMsg pfnWSARecvMsg;
	};
}

#endif
