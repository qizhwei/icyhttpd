#include "Socket.h"
#include "Types.h"
#include "SocketProvider.h"
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
			return SocketProvider::Instance().AcceptEx()(
				this->hSocket, this->hAcceptSocket, buffer,
				0, AddressLength, AddressLength, NULL, this)
				|| WSAGetLastError() == ERROR_IO_PENDING;
		}

	private:
		SOCKET hSocket;
		SOCKET hAcceptSocket;
		char buffer[AddressLength * 2];
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
			return !WSARecv(this->hSocket, &this->WSABuf, 1, NULL, &dwFlags, this, NULL)
				|| WSAGetLastError() == ERROR_IO_PENDING;
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
			return !WSASend(this->hSocket, &this->WSABuf, 1, NULL, 0, this, NULL)
				|| WSAGetLastError() == ERROR_IO_PENDING;
		}

	private:
		SOCKET hSocket;
		WSABUF WSABuf;
	};

	class WriteOperation2: public OverlappedOperation
	{
	public:
		WriteOperation2(SOCKET hSocket, WSABUF *WSABuf, UInt32 count)
			: hSocket(hSocket), WSABuf(WSABuf), count(count)
		{}

		virtual bool operator()()
		{
			return !WSASend(this->hSocket, this->WSABuf, this->count, NULL, 0, this, NULL)
				|| WSAGetLastError() == ERROR_IO_PENDING;
		}

	private:
		SOCKET hSocket;
		WSABUF *WSABuf;
		UInt32 count;
	};

	class TransmitFileOperation: public OverlappedOperation
	{
	public:
		TransmitFileOperation(SOCKET hSocket, HANDLE hFile, UInt64 offset, UInt32 size)
			: hSocket(hSocket), hFile(hFile), OverlappedOperation(offset), size(size)
		{}

		virtual bool operator()()
		{
			return SocketProvider::Instance().TransmitFile()(
				this->hSocket, this->hFile, size, 0, this, NULL, 0)
				|| WSAGetLastError() == ERROR_IO_PENDING;
		}

	private:
		SOCKET hSocket;
		HANDLE hFile;
		UInt32 size;
	};
}

namespace Httpd
{
	Socket::Socket()
		: hSocket(SocketProvider::Instance().Create())
	{
		try {
			// TODO: Is it always safe to treat SOCKET as HANDLE?
			Dispatcher::Instance().BindHandle((HANDLE)this->hSocket, OverlappedOperationKey);
		} catch (...) {
			SocketProvider::Instance().Destroy(this->hSocket);
			throw;
		}
	}

	Socket::~Socket()
	{
		SocketProvider::Instance().Destroy(this->hSocket);
	}

	void Socket::BindIP(const char *ip, UInt16 port)
	{
		struct sockaddr_in service;
		service.sin_family = AF_INET;
		service.sin_port = htons(port);
		service.sin_addr.s_addr = inet_addr(ip);

		if (bind(this->hSocket, (SOCKADDR *)&service, sizeof(service)))
			throw SystemException();
	}

	void Socket::Listen(int backlog)
	{
		if (listen(this->hSocket, backlog))
			throw SystemException();
	}

	void Socket::Accept(Socket &acceptSocket)
	{
		AcceptOperation operation(this->hSocket, acceptSocket.hSocket);
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation);
	}

	UInt32 Socket::Read(char *buffer, UInt32 size)
	{
		ReadOperation operation(this->hSocket, buffer, size);
		return Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation);
	}

	void Socket::Write(const char *buffer, UInt32 size)
	{
		WriteOperation operation(this->hSocket, buffer, size);
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation);
	}

	void Socket::Write(WSABUF *WSABuf, UInt32 count)
	{
		WriteOperation2 operation(this->hSocket, WSABuf, count);
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation);
	}

	void Socket::TransmitFile(HANDLE hFile, UInt64 offset, UInt32 size)
	{
		TransmitFileOperation operation(this->hSocket, hFile, offset, size);
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation);
	}
}
