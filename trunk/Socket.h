#ifndef _SOCKET_H
#define _SOCKET_H

#include "Types.h"
#include "Dispatcher.h"
#include "Win32.h"

namespace Httpd
{
	class Socket: NonCopyable
	{
	public:
		Socket();
		virtual ~Socket();

		void BindIP(const char *ip, UInt16 port);
		// TODO: BindIPv6. But inet_pton is not implemented before Windows Vista.

		void Listen(int backlog = SOMAXCONN);
		void Accept(Socket &acceptSocket);
		void Disconnect(bool reuse);
		UInt32 Read(char *buffer, UInt32 size);
		void Write(const char *buffer, UInt32 size);
		void Write(const char *buffer0, UInt32 size0, const char *buffer1, UInt32 size1);
		void TransmitFile(HANDLE hFile, UInt64 offset = 0, UInt32 size = 0);

	private:
		SOCKET hSocket;
		bool canReuse;
	};
}

#endif
