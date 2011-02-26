#include "Socket.h"
#include "Types.h"
#include "SocketPool.h"
#include "Dispatcher.h"
#include "Constant.h"
#include "Exception.h"
#include <cstring>

namespace Httpd
{
	Socket::Socket()
		: hSocket(SocketPool::Instance().Pop()), canReuse(true)
	{
		try {
			// TODO: Is it always safe to treat SOCKET as HANDLE?
			Dispatcher::Instance().BindHandle((HANDLE)this->hSocket, OverlappedOperationKey);
		} catch (...) {
			SocketPool::Instance().Push(this->hSocket, true);
			throw;
		}
	}

	Socket::~Socket()
	{
		SocketPool::Instance().Push(this->hSocket, this->canReuse);
	}

	void Socket::BindIP(const char *ip, UInt16 port)
	{
		struct sockaddr_in service;
		service.sin_family = AF_INET;
		service.sin_port = htons(port);
		service.sin_addr.s_addr = inet_addr(ip);

		this->canReuse = false;
		if (bind(this->hSocket, (SOCKADDR *)&service, sizeof(service)))
			throw Exception();
	}

	void Socket::Listen(int backlog)
	{
		this->canReuse = false;
		if (listen(this->hSocket, backlog))
			throw Exception();
	}

	void Socket::Accept(Socket &acceptSocket)
	{
		const int AddressLength = sizeof(sockaddr_in6) + 16;
		char buffer[AddressLength * 2];
		OverlappedOperation overlapped;

		this->canReuse = false;
		if (!SocketPool::Instance().AcceptEx(this->hSocket, acceptSocket.hSocket, buffer,
			0, AddressLength, AddressLength, NULL, &overlapped)
			&& WSAGetLastError() != ERROR_IO_PENDING)
			throw Exception();
		acceptSocket.canReuse = false;

		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), overlapped);
	}

	void Socket::Disconnect(bool reuse)
	{
		OverlappedOperation overlapped;

		if (!SocketPool::Instance().DisconnectEx(this->hSocket, &overlapped,
			reuse ? TF_REUSE_SOCKET : 0, 0) && WSAGetLastError() != ERROR_IO_PENDING)
		{
			this->canReuse = false;
			throw Exception();
		}
		this->canReuse = reuse;

		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), overlapped);
	}

	UInt32 Socket::Read(char *buffer, UInt32 size)
	{
		WSABUF WSABuf;
		WSABuf.buf = buffer;
		WSABuf.len = size;

		DWORD dwFlags = 0;
		OverlappedOperation overlapped;

		this->canReuse = false;
		if (WSARecv(this->hSocket, &WSABuf, 1, NULL, &dwFlags, &overlapped, NULL)
			&& WSAGetLastError() != ERROR_IO_PENDING)
			throw Exception();

		return Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), overlapped);
	}

	void Socket::Write(const char *buffer, UInt32 size)
	{
		WSABUF WSABuf;
		WSABuf.buf = const_cast<char *>(buffer);
		WSABuf.len = size;

		OverlappedOperation overlapped;

		this->canReuse = false;
		if (WSASend(this->hSocket, &WSABuf, 1, NULL, 0, &overlapped, NULL)
			&& WSAGetLastError() != ERROR_IO_PENDING)
			throw Exception();

		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), overlapped);
	}
}
