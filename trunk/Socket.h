#ifndef _SOCKET_H
#define _SOCKET_H

#include "Types.h"
#include "Win32.h"

namespace Httpd
{
	class Socket: NonCopyable
	{
	public:
		Socket();
		~Socket();

		void CreateListenerIpv4(const char *ip, UInt16 port);
		void CreateClientIpv4();
		void Listen(int backlog = SOMAXCONN);
		void Accept(Socket &acceptSocket);
		UInt32 Read(char *buffer, UInt32 size);
		void Write(const char *buffer, UInt32 size);
		void Write(WSABUF *WSABuf, UInt32 count);
		void TransmitFile(HANDLE hFile, UInt64 offset, UInt32 size);

	private:
		SOCKET hSocket;
	};
}

#endif
