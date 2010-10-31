#include "rtl.h"
#include <stdio.h>

static int PipeCount = 0;
static const char PipeNameFormat[] = "\\\\.\\pipe\\icyhttpd\\critter.%08x.%08x";

int RtlCreatePipe(HANDLE *pipeServer, HANDLE *pipeClient)
{
	char buffer[48];
	HANDLE pipe0, pipe1;

	sprintf(buffer, PipeNameFormat, GetCurrentProcessId(), ++PipeCount);

	// Create the server end of the pipe
	pipe0 = CreateNamedPipeA(buffer, FILE_FLAG_OVERLAPPED | PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_READMODE_BYTE, PIPE_UNLIMITED_INSTANCES,
		4096, 4096, 0, NULL);

	if (pipe0 == INVALID_HANDLE_VALUE) {
		return 1;
	}

	// Open the client end of the pipe
	pipe1 = CreateFileA(buffer, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	if (pipe1 == INVALID_HANDLE_VALUE) {
		CloseHandle(pipe0);
		return 1;
	}

	*pipeServer = pipe0;
	*pipeClient = pipe1;
	return 0;
}