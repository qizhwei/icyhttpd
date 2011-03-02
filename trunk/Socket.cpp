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
			return SocketPool::Instance().AcceptEx()(
				this->hSocket, this->hAcceptSocket, buffer,
				0, AddressLength, AddressLength, NULL, this)
				|| WSAGetLastError() == ERROR_IO_PENDING;
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
			return SocketPool::Instance().DisconnectEx()(
				this->hSocket, this, this->reuse ? TF_REUSE_SOCKET : 0, 0)
				|| WSAGetLastError() == ERROR_IO_PENDING;
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
		WriteOperation2(SOCKET hSocket, const char *buffer0, UInt32 size0, const char *buffer1, UInt32 size1)
			: hSocket(hSocket)
		{
			WSABuf[0].buf = const_cast<char *>(buffer0);
			WSABuf[0].len = size0;
			WSABuf[1].buf = const_cast<char *>(buffer1);
			WSABuf[1].len = size1;
		}

		virtual bool operator()()
		{
			return !WSASend(this->hSocket, &this->WSABuf[0], 2, NULL, 0, this, NULL)
				|| WSAGetLastError() == ERROR_IO_PENDING;
		}

	private:
		SOCKET hSocket;
		WSABUF WSABuf[2];
	};

	class TransmitFileOperation: public OverlappedOperation
	{
	public:
		TransmitFileOperation(SOCKET hSocket, HANDLE hFile, UInt64 offset, UInt32 size)
			: hSocket(hSocket), hFile(hFile), OverlappedOperation(offset), size(size)
		{}

		virtual bool operator()()
		{
			return SocketPool::Instance().TransmitFile()(
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
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation);
		acceptSocket.canReuse = false;
	}

	void Socket::Disconnect(bool reuse)
	{
		DisconnectOperation operation(this->hSocket, reuse);
		this->canReuse = false;
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation);
		this->canReuse = reuse;
	}

	UInt32 Socket::Read(char *buffer, UInt32 size)
	{
		ReadOperation operation(this->hSocket, buffer, size);
		this->canReuse = false;
		return Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation);
	}

	void Socket::Write(const char *buffer, UInt32 size)
	{
		WriteOperation operation(this->hSocket, buffer, size);
		this->canReuse = false;
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation);
	}

	void Socket::Write(const char *buffer0, UInt32 size0, const char *buffer1, UInt32 size1)
	{
		WriteOperation2 operation(this->hSocket, buffer0, size0, buffer1, size1);
		this->canReuse = false;
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation);
	}

	void Socket::TransmitFile(HANDLE hFile, UInt64 offset, UInt32 size)
	{
		TransmitFileOperation operation(this->hSocket, hFile, offset, size);
		this->canReuse = false;
		Dispatcher::Instance().Block(reinterpret_cast<HANDLE>(this->hSocket), operation);
	}
}
