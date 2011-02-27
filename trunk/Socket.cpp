#include "Socket.h"
#include "Types.h"
#include "SocketPool.h"
#include "Dispatcher.h"
#include "Constant.h"
#include "Exception.h"

using namespace Httpd;
using namespace std;

namespace
{
	const int AddressLength = sizeof(sockaddr_in6) + 16;

	class AcceptOperation: public OverlappedOperation
	{
	public:
		AcceptOperation(SOCKET hSocket, SOCKET hAcceptSocket)
			: hSocket(hSocket), hAcceptSocket(hAcceptSocket)
		{}

		virtual bool operator()()
		{
			return static_cast<bool>(SocketPool::Instance().AcceptEx()(
				this->hSocket, this->hAcceptSocket, buffer,
				0, AddressLength, AddressLength, NULL, this)
				|| WSAGetLastError() == ERROR_IO_PENDING);
		}

	private:
		SOCKET hSocket;
		SOCKET hAcceptSocket;
		char buffer[AddressLength * 2];
	};

	class DisconnectOperation: public OverlappedOperation
	{
	public:
		DisconnectOperation(SOCKET hSocket, bool reuse)
			: hSocket(hSocket), reuse(reuse)
		{}

		virtual bool operator()()
		{
			return static_cast<bool>(SocketPool::Instance().DisconnectEx()(
				this->hSocket, this, this->reuse ? TF_REUSE_SOCKET : 0, 0)
				|| WSAGetLastError() == ERROR_IO_PENDING);
		}

	private:
		SOCKET hSocket;
		bool reuse;
	};

	class ReadOperation: public OverlappedOperation
	{
	public:
		ReadOperation(SOCKET hSocket, char *buffer, UInt32 size)
			: hSocket(hSocket)
		{
			WSABuf.buf = buffer;
			WSABuf.len = size;
		}

		virtual bool operator()()
		{
			DWORD dwFlags = 0;
			return static_cast<bool>(!WSARecv(this->hSocket, &this->WSABuf, 1, NULL,
				&dwFlags, this, NULL) || WSAGetLastError() == ERROR_IO_PENDING);
		}

	private:
		SOCKET hSocket;
		WSABUF WSABuf;
	};

	class WriteOperation: public OverlappedOperation
	{
	public:
		WriteOperation(SOCKET hSocket, const char *buffer, UInt32 size)
			: hSocket(hSocket)
		{
			WSABuf.buf = const_cast<char *>(buffer);
			WSABuf.len = size;
		}

		virtual bool operator()()
		{
			return static_cast<bool>(!WSASend(this->hSocket, &this->WSABuf, 1, NULL,
				0, this, NULL) || WSAGetLastError() == ERROR_IO_PENDING);
		}

	private:
		SOCKET hSocket;
		WSABUF WSABuf;
	};
}

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
			throw SystemException();
	}

	void Socket::Listen(int backlog)
	{
		this->canReuse = false;
		if (listen(this->hSocket, backlog))
			throw SystemException();
	}

	void Socket::Accept(Socket &acceptSocket)
	{
		AcceptOperation operation(this->hSocket, acceptSocket.hSocket);

		this->canReuse = false;
		if (Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation) == -1)
			throw SystemException();
		acceptSocket.canReuse = false;
	}

	void Socket::Disconnect(bool reuse)
	{
		DisconnectOperation operation(this->hSocket, reuse);

		if (Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation) == -1) {
			this->canReuse = false;
			throw SystemException();
		}
		this->canReuse = reuse;
	}

	UInt32 Socket::Read(char *buffer, UInt32 size)
	{
		ReadOperation operation(this->hSocket, buffer, size);

		this->canReuse = false;
		Int32 result = Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation);
		if (result == -1)
			throw SystemException();
		return static_cast<UInt32>(result);
	}

	void Socket::Write(const char *buffer, UInt32 size)
	{
		WriteOperation operation(this->hSocket, buffer, size);

		this->canReuse = false;
		if (Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation) == -1)
			throw SystemException();
	}
}
