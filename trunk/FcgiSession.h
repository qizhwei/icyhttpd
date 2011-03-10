#ifndef _FCGISESSION_H
#define _FCGISESSION_H

#include "Types.h"
#include "Stream.h"
#include "Pipe.h"

namespace Httpd
{
	class FcgiProcess;

	class FcgiSession: NonCopyable
	{
	public:
		FcgiSession(FcgiProcess &process);

		// Whether or not the fcgi process can be reused.
		bool KeepAlive();

		// Whether or not error occurred
		bool HasError();

		// N.B. The write operations are NOT designed thread-safe,
		// two threads writing on the same session may violate the pipe.

		// Write to param stream
		void WriteParam(const char *key, const char *value);
		void FlushParam();
		void CloseParam();

		// Write to stdin
		void Write(const char *buffer, UInt32 size);

		// Read from stdout
		UInt32 Read(char *buffer, UInt32 size);

	private:
		FcgiProcess &process;
		LocalPipe stdoutPipe;
		UInt16 requestId;
		bool hasError;
		std::vector<char> paramBuffer;

		void InitializeParamBuffer();
		void PrepareParamBufferForSending();
		void PushParamBuffer(const char *buffer, UInt32 size);

		static void ReadProc(void *param);
	};
}

#endif
