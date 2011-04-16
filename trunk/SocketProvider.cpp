#include "SocketProvider.h"
#include "Win32.h"
#include "Exception.h"

using namespace Httpd;
using namespace std;

namespace
{
	// We had to use hardcode here since MinGW32 doesn't provide these information
	const GUID guidTransmitFile = {0xb5367df0,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}};
	const GUID guidAcceptEx = {0xb5367df1,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}};
	const GUID guidGetAcceptExSockaddrs = {0xb5367df2,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}};
	const GUID guidTransmitPackets = {0xd9689da0,0x1f90,0x11d3,{0x99,0x71,0x00,0xc0,0x4f,0x68,0xc8,0x76}};
	const GUID guidConnectEx = {0x25a207b9,0xddf3,0x4660,{0x8e,0xe9,0x76,0xe5,0x8c,0x74,0x06,0x3e}};
	const GUID guidDisconnectEx = {0x7fda2e11,0x8630,0x436f,{0xa0, 0x31, 0xf5, 0x36, 0xa6, 0xee, 0xc1, 0x57}};
	const GUID guidWSARecvMsg = {0xf689d7c8,0x6f1f,0x436b,{0x8a,0x53,0xe5,0x4f,0xe3,0x51,0xc3,0x22}};

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

		SOCKET s = this->Create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		GetFunctionPointer(s, &guidTransmitFile, &this->pfnTransmitFile);
		GetFunctionPointer(s, &guidAcceptEx, &this->pfnAcceptEx);
		GetFunctionPointer(s, &guidGetAcceptExSockaddrs, &this->pfnGetAcceptExSockaddrs);
		GetFunctionPointer(s, &guidTransmitPackets, &this->pfnTransmitPackets);
		GetFunctionPointer(s, &guidConnectEx, &this->pfnConnectEx);
		GetFunctionPointer(s, &guidDisconnectEx, &this->pfnDisconnectEx);
		GetFunctionPointer(s, &guidWSARecvMsg, &this->pfnWSARecvMsg);
		this->Destroy(s);
	}

	SOCKET SocketProvider::Create(int af, int type, int protocol)
	{
		SOCKET s;
		if ((s = socket(af, type, protocol)) == INVALID_SOCKET)
			throw SystemException();
		return s;
	}

	void SocketProvider::Destroy(SOCKET s)
	{
		closesocket(s);
	}
}
