#include "Fastcgi.h"
#include "Types.h"

using namespace Httpd;

namespace
{
	inline unsigned char LoByte(UInt16 i)
	{
		return static_cast<unsigned char>(i);
	}

	inline unsigned char HiByte(UInt16 i)
	{
		return i >> 8;
	}

	inline UInt16 MakeWord(unsigned char loByte, unsigned char hiByte)
	{
		return static_cast<UInt16>(loByte) | (static_cast<UInt16>(hiByte) << 8);
	}
}

namespace Httpd
{
	FcgiHeader::FcgiHeader(unsigned char version, unsigned char type, UInt16 requestId,
		UInt16 contentLength, unsigned char paddingLength)
		: version(version), type(type), requestIdB1(HiByte(requestId)), requestIdB0(LoByte(requestId))
		, contentLengthB1(HiByte(contentLength)), contentLengthB0(LoByte(contentLength))
		, paddingLength(paddingLength)
	{}

	UInt16 FcgiHeader::RequestId() const
	{
		return MakeWord(this->requestIdB0, this->requestIdB1);
	}

	UInt16 FcgiHeader::ContentLength() const
	{
		return MakeWord(this->contentLengthB0, this->contentLengthB1);
	}

	FcgiBeginRequestRecord::FcgiBeginRequestRecord(UInt16 requestId, UInt16 role, unsigned char flags)
		: header(FcgiVersion1, FcgiBeginRequest, requestId, 8, 0)
		, roleB1(HiByte(role)), roleB0(LoByte(role)), flags(flags)
	{
	}
}
