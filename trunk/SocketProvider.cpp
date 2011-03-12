#include "SocketProvider.h"
#include "Win32.h"
#include "Exception.h"

using namespace Httpd;
using namespace std;

namespace
{
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
			throw FatalException();
	}
}

namespace Httpd
{
	SocketProvider &SocketProvider::Instance()
	{
		static SocketProvider *f(new SocketProvider());
		return *f;
	}

	SocketProvider::SocketProvider()
	{
		WSADATA WSAData;

		if (WSAStartup(MAKEWORD(2, 2), &WSAData))
			throw FatalException();

		SOCKET s = this->Create();
		GetFunctionPointer(s, &guidTransmitFile, &this->pfnTransmitFile);
		GetFunctionPointer(s, &guidAcceptEx, &this->pfnAcceptEx);
		GetFunctionPointer(s, &guidGetAcceptExSockaddrs, &this->pfnGetAcceptExSockaddrs);
		GetFunctionPointer(s, &guidTransmitPackets, &this->pfnTransmitPackets);
		GetFunctionPointer(s, &guidConnectEx, &this->pfnConnectEx);
		GetFunctionPointer(s, &guidDisconnectEx, &this->pfnDisconnectEx);
		GetFunctionPointer(s, &guidWSARecvMsg, &this->pfnWSARecvMsg);
		this->Destroy(s);
	}

	SOCKET SocketProvider::Create()
	{
		SOCKET s;
		if ((s = socket(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
			throw SystemException();
		return s;
	}

	void SocketProvider::Destroy(SOCKET s)
	{
		closesocket(s);
	}
}
