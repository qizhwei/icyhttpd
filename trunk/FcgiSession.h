#ifndef _FCGISESSION_H
#define _FCGISESSION_H

#include "Types.h"
#include "Stream.h"
#include "Pipe.h"
#include "Utility.h"
#include <memory>

namespace Httpd
{
	class FcgiProcess;
	struct FcgiSessionContext;
	typedef std::shared_ptr<FcgiSessionContext> FcgiSessionContextPtr;

	class FcgiSession: NonCopyable
	{
	public:
		FcgiSession(FcgiProcess &process);
		~FcgiSession();

		// Whether or not the fcgi process can be reused.
		bool KeepAlive() const;

		// Whether or not error occurred
		bool HasError() const;

		// N.B. The write operations are NOT designed thread-safe,
		// two threads writing on the same session may violate the pipe.

		// Write to param stream
		void WriteParam(const char *key, const char *value);
		void FlushParam();
		void CloseParam();

		// Write to stdin
		void Write(const char *buffer, UInt32 size);
		void CloseStdin();

		// Read from stdout
		UInt32 Read(char *buffer, UInt32 size);

	private:
		FcgiSessionContextPtr session;
		std::vector<char> paramBuffer;

	private:
		void InitializeParamBuffer();
		void PrepareParamBufferForSending();
		void PushParamBuffer(const char *buffer, UInt32 size);
	};
}

#endif
