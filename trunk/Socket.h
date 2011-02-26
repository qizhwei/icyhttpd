#ifndef _SOCKET_H
#define _SOCKET_H

#include "Types.h"
#include "Dispatcher.h"
#include "Win32.h"

namespace Httpd
{
	class Socket: NonCopyable, public Readable, public Writable
	{
	public:
		Socket();
		virtual ~Socket();

		void Abort(bool canReuse = false);
		void BindIP(const char *ip, UInt16 port);
		// TODO: BindIPv6. But inet_pton is not implemented before Windows Vista.

		void Listen(int backlog = SOMAXCONN);
		void Accept(Socket &acceptSocket);
		void Disconnect(bool reuse);
		virtual UInt32 Read(char *buffer, UInt32 size);
		virtual void Write(const char *buffer, UInt32 size);

	private:
		SOCKET hSocket;
	};
}

#endif
