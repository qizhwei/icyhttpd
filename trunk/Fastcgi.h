#ifndef _FASTCGI_H
#define _FASTCGI_H

#include "Types.h"

namespace Httpd
{
	// Value for version component of FcgiHeader
	const unsigned char FcgiVersion1 = 1;

	// Values for type component of FcgiHeader
	const unsigned char FcgiBeginRequest = 1;
	const unsigned char FcgiAbortRequest = 2;
	const unsigned char FcgiEndRequest = 3;
	const unsigned char FcgiParams = 4;
	const unsigned char FcgiStdin = 5;
	const unsigned char FcgiStdout = 6;
	const unsigned char FcgiStderr = 7;
	const unsigned char FcgiData = 8;
	const unsigned char FcgiGetValues = 9;
	const unsigned char FcgiGetValuesResult = 10;
	const unsigned char FcgiUnknownType = 11;
	const unsigned char FcgiMaxType = FcgiUnknownType;

	// Mask for flags component of FCGI_BeginRequestBody
	const unsigned char FcgiKeepConn = 1;

	// Values for role component of FCGI_BeginRequestBody
	const UInt16 FcgiResponder = 1;
	const UInt16 FcgiAuthorizer = 2;
	const UInt16 FcgiFilter = 3;

	class FcgiHeader: public POD
	{
	public:
		FcgiHeader() {}
		FcgiHeader(unsigned char version, unsigned char type, UInt16 requestId,
			UInt16 contentLength, unsigned char paddingLength);

		unsigned char Version() const { return version; }
		unsigned char Type() const { return type; }
		UInt16 RequestId() const;
		UInt16 ContentLength() const;
		unsigned char PaddingLength() const { return paddingLength; }

	private:
		unsigned char version;
		unsigned char type;
		unsigned char requestIdB1;
		unsigned char requestIdB0;
		unsigned char contentLengthB1;
		unsigned char contentLengthB0;
		unsigned char paddingLength;
		unsigned char reserved;
	};

	static_assert(sizeof(FcgiHeader) == 8, "sizeof(FcgiHeader) should be 8");

	class FcgiBeginRequestRecord: public POD
	{
	public:
		FcgiBeginRequestRecord(UInt16 requestId, UInt16 role, unsigned char flags);

	private:
		FcgiHeader header;
		unsigned char roleB1;
		unsigned char roleB0;
		unsigned char flags;
		unsigned char reserved[5];
	};

	static_assert(sizeof(FcgiBeginRequestRecord) == 16, "sizeof(FcgiBeginRequestRecord) should be 16");
}

#endif
