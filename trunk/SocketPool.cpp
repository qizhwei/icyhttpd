#include "SocketPool.h"
#include "Win32.h"
#include "Exception.h"

namespace
{
	using namespace Httpd;

	const GUID guidTransmitFile = WSAID_TRANSMITFILE;
	const GUID guidAcceptEx = WSAID_ACCEPTEX;
	const GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	const GUID guidTransmitPackets = WSAID_TRANSMITPACKETS;
	const GUID guidConnectEx = WSAID_CONNECTEX;
	const GUID guidDisconnectEx = WSAID_DISCONNECTEX;
	const GUID guidWSARecvMsg = WSAID_WSARECVMSG;

	template <typename FunctionPointer>
	void GetFunctionPointer(SOCKET s, const GUID *g, FunctionPointer *pfn)
	{
		DWORD dwBytes;

		if (WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, const_cast<GUID *>(g), sizeof(*g),
			pfn, sizeof(FunctionPointer), &dwBytes, NULL, NULL))
			throw SystemException();
	}
}

namespace Httpd
{
	SocketPool &SocketPool::Instance()
	{
		static SocketPool *f(new SocketPool());
		return *f;
	}

	SocketPool::SocketPool()
	{
		WSADATA WSAData;

		if (WSAStartup(MAKEWORD(2, 2), &WSAData))
			throw FatalException();

		try {
			SOCKET s = this->Pop();
			GetFunctionPointer(s, &guidTransmitFile, &this->TransmitFile);
			GetFunctionPointer(s, &guidAcceptEx, &this->AcceptEx);
			GetFunctionPointer(s, &guidGetAcceptExSockaddrs, &this->GetAcceptExSockaddrs);
			GetFunctionPointer(s, &guidTransmitPackets, &this->TransmitPackets);
			GetFunctionPointer(s, &guidConnectEx, &this->ConnectEx);
			GetFunctionPointer(s, &guidDisconnectEx, &this->DisconnectEx);
			GetFunctionPointer(s, &guidWSARecvMsg, &this->WSARecvMsg);
			this->Push(s, true);
		} catch (...) {
			throw FatalException();
		}
	}

	// TODO: Implement a socket pool, take good care of multithread synchronizing (and performance)
	SOCKET SocketPool::Pop()
	{
		SOCKET s = socket(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP);
		if (s == INVALID_SOCKET)
			throw SystemException();
		return s;
	}

	void SocketPool::Push(SOCKET s, bool canReuse)
	{
		closesocket(s);
	}
}
