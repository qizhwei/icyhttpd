#include "FcgiSession.h"
#include "FcgiProcess.h"
#include "Fastcgi.h"
#include "Utility.h"
#include "Dispatcher.h"
#include "Constant.h"
#include "Exception.h"
#include <cstring>
#include <algorithm>

using namespace Httpd;
using namespace std;

namespace
{
	const UInt32 MaxPacketSize = 0x1000;
	const UInt32 MaxContentSize = MaxPacketSize - sizeof(FcgiHeader);
}

namespace Httpd
{
	void FcgiSession::ReadProc(void *param)
	{
		SharedPtr<FcgiSession> fs(static_cast<FcgiSession *>(param));
		FcgiHeader header;
		char buffer[BufferBlockSize];
		bool ended = false;

		try {
			do {
				// Read header
				if (fs->process.Read(header.Buffer(), sizeof(header)) == 0)
					break;

				if (header.Version() != FcgiVersion1 || header.RequestId() != fs->requestId)
					break;

				UInt32 remainingLength = header.ContentLength();

				// Dispatch data
				switch (header.Type()) {
				case FcgiStdout:
					if (remainingLength == 0) {
						fs->stdoutPipe.CloseWrite();
					} else {
						do {
							UInt32 readSize = std::min(remainingLength, BufferBlockSize);
							if (fs->process.Read(buffer, readSize) == 0)
								goto bed;
							remainingLength -= readSize;
							fs->stdoutPipe.Write(buffer, readSize);
						} while (remainingLength != 0);
					}
					break;
				case FcgiEndRequest:
					ended = true;
					break;
				}

				// Read remaining data
				remainingLength += header.PaddingLength();
				while (remainingLength != 0) {
					UInt32 readSize = std::min(remainingLength, BufferBlockSize);
					if (fs->process.Read(buffer, readSize) == 0)
						goto bed;
					remainingLength -= readSize;
				}
			} while (!ended);
		} catch (const std::exception &) {
		}
bed:
		if (!ended)
			fs->hasError = true;
		fs->stdoutPipe.CloseWrite();
	}

	FcgiSession::FcgiSession(FcgiProcess &process)
		: process(process), stdoutPipe(CreatePipePair())
		, requestId(process.Acquire()), hasError(false)
	{
		this->InitializeParamBuffer();
		Dispatcher::Instance().Queue(&this->ReadProc, this->AddRef());
	}

	bool FcgiSession::KeepAlive()
	{
		return this->requestId != 0 && this->hasError == false;
	}

	bool FcgiSession::HasError()
	{
		return this->hasError;
	}

	void FcgiSession::InitializeParamBuffer()
	{
		this->paramBuffer.resize(sizeof(FcgiHeader));
	}

	void FcgiSession::FlushParam()
	{
		if (this->paramBuffer.size() != sizeof(FcgiHeader)) {
			this->PrepareParamBufferForSending();
			this->process.Write(&*this->paramBuffer.begin(), this->paramBuffer.size());
			this->InitializeParamBuffer();
		}
	}

	void FcgiSession::CloseParam()
	{
		this->FlushParam();
		this->InitializeParamBuffer();
		this->PrepareParamBufferForSending();
		this->process.Write(&*this->paramBuffer.begin(), this->paramBuffer.size());
		vector<char>().swap(this->paramBuffer);
	}

	void FcgiSession::PrepareParamBufferForSending()
	{
		const size_t currentSize = this->paramBuffer.size();
		const UInt16 contentLength = currentSize - sizeof(FcgiHeader);
		const unsigned char paddingLength = 7 - ((contentLength + 7) & 7);
		FcgiHeader header(FcgiVersion1, FcgiParams, this->requestId, contentLength, paddingLength);
		memcpy(&*this->paramBuffer.begin(), header.Buffer(), sizeof(FcgiHeader));
		this->paramBuffer.resize(currentSize + paddingLength);
	}

	void FcgiSession::PushParamBuffer(const char *buffer, UInt32 size)
	{
		while (true) {
			const size_t currentSize = this->paramBuffer.size();
			const UInt32 remainingLength = MaxPacketSize - currentSize;

			if (size < remainingLength) {
				this->paramBuffer.resize(currentSize + size);
				memcpy(&this->paramBuffer[currentSize], buffer, size);
				break;
			} else {
				this->paramBuffer.resize(MaxPacketSize);
				memcpy(&this->paramBuffer[currentSize], buffer, remainingLength);
				this->FlushParam();
				this->InitializeParamBuffer();
				buffer += remainingLength;
				size -= remainingLength;
			}
		}
	}

	void FcgiSession::WriteParam(const char *key, const char *value)
	{
		const size_t keyLength = strlen(key);
		const size_t valueLength = strlen(value);
		char buffer[8];
		UInt32 offset = 0;

		if (keyLength < 0x80) {
			buffer[offset++] = keyLength;
		} else {
			buffer[offset++] = 0x80 | (keyLength >> 24);
			buffer[offset++] = keyLength >> 16;
			buffer[offset++] = keyLength >> 8;
			buffer[offset++] = keyLength;
		}

		if (valueLength < 0x80) {
			buffer[offset++] = valueLength;
		} else {
			buffer[offset++] = 0x80 | (valueLength >> 24);
			buffer[offset++] = valueLength >> 16;
			buffer[offset++] = valueLength >> 8;
			buffer[offset++] = valueLength;
		}

		this->PushParamBuffer(buffer, offset);
		this->PushParamBuffer(key, keyLength);
		this->PushParamBuffer(value, valueLength);
	}

	UInt32 FcgiSession::Read(char *buffer, UInt32 size)
	{
		return this->stdoutPipe.Read(buffer, size);
	}

	void FcgiSession::Write(const char *buffer, UInt32 size)
	{
		while (size != 0) {
			const UInt16 contentLength = std::min(size, MaxContentSize);
			const unsigned char paddingLength = 7 - ((contentLength + 7) & 7);

			// Write header
			FcgiHeader header(FcgiVersion1, FcgiStdin, this->requestId, contentLength, paddingLength);
			this->process.Write(header.Buffer(), sizeof(header));

			// Write data
			this->process.Write(buffer, contentLength);

			// Write padding
			this->process.Write(header.Buffer(), paddingLength);
			
			buffer += contentLength;
			size -= contentLength;
		}
	}

	void FcgiSession::CloseStdin()
	{
		FcgiHeader header(FcgiVersion1, FcgiStdin, this->requestId, 0, 0);
		this->process.Write(header.Buffer(), sizeof(header));
	}
};
