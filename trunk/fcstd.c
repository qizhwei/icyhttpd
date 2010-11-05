#include "fcp.h"
#include "rtl.h"
#include <assert.h>

typedef struct _FcpReadPipeState {
	OVERLAPPED Overlapped;
	FcProcess *Process;
	size_t Length;
	unsigned char Buffer[FCGI_HEADER_LEN];
} FcpReadPipeState;

static void CALLBACK FcpReadPipeComplete(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped);
static void FcpWriteStdoutCompletion(void *state, size_t size, int error);
static void FcpWriteStdoutEofCompletion(void *state, size_t size, int error);

int FcpReadPipe(FcProcess *process)
{
	FcpReadPipeState *rpstate;
	
	// Allocate memory for asynchronous state object
	rpstate = RtlAllocateHeap(sizeof(FcpReadPipeState));
	if (rpstate == NULL) {
		return 1;
	}
	
	// Initialize the state object
	memset(&rpstate->Overlapped, 0, sizeof(OVERLAPPED));
	rpstate->Process = ObReferenceObjectByPointer(process, NULL);
	rpstate->Length = 0;
	
	// Begin read operation
	if (!ReadFileEx(process->LocalPipe, rpstate->Buffer, FCGI_HEADER_LEN,
		&rpstate->Overlapped, &FcpReadPipeComplete))
	{
		RtlFreeHeap(rpstate);
		ObDereferenceObject(process);
		return 1;
	}
	
	return 0;
}

static void CALLBACK FcpReadPipeComplete(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped)
{
	FcpReadPipeState *rpstate = (FcpReadPipeState *)overlapped;
	FcProcess *process = rpstate->Process;
	FcRequest *request;
	FCGI_Header *header;
	size_t contentLength, contentSize;
	
	if (errorCode != 0) {
		RtlFreeHeap(rpstate);
		FcpTerminateProcess(process, 1);
		ObDereferenceObject(process);
		return;
	}
	
	rpstate->Length += bytesTransferred;
	
	if (rpstate->Length >= FCGI_HEADER_LEN) {
		header = (FCGI_Header *)&rpstate->Buffer[0];
		contentLength = (header->contentLengthB1 << 8) + header->contentLengthB0;
		contentSize = contentLength + header->paddingLength;
	} else {
		contentSize = 0;
	}
	
	assert(rpstate->Length <= FCGI_HEADER_LEN + contentSize);
	
	if (rpstate->Length == FCGI_HEADER_LEN + contentSize) {
		
		// We have received a complete packet
		if (process->State != FCP_STATE_INTERACTIVE) {
			goto Error;
		}
		
		request = process->Request;
		
		switch (header->type) {
			case FCGI_STDOUT:
				if (RtlWriteFifo(request->StdoutFifo, &rpstate->Buffer[FCGI_HEADER_LEN],
					 contentLength, &FcpWriteStdoutCompletion, rpstate))
				{
					goto Error;
				}
				
				// Since the writing operation has borrowed the state, we must create a new one
				rpstate = NULL;
				break;
			case FCGI_END_REQUEST:
				if (RtlWriteFifo(request->StdoutFifo, NULL, 0, &FcpWriteStdoutEofCompletion, NULL)) {
					goto Error;
				}
				process->State = FCP_STATE_READY;
				ObDereferenceObject(request);
				process->RemainingRequests -= 1;
				if (process->RemainingRequests == 0 || FcpPushPoolProcess(process)) {
					goto Error;
				}
				break;
		}
		
		// Start to read the next packet
		if (RtlReallocateHeap(&rpstate, sizeof(FcpReadPipeState) + contentSize)) {
			goto Error;
		}
		
		// Since the rpstate might be a brand new buffer, we must reinitialize it all over
		// Leave the reference count of the process unchanged
		memset(&rpstate->Overlapped, 0, sizeof(OVERLAPPED));
		rpstate->Process = process;
		rpstate->Length = 0;
		
		// Begin read operation
		if (!ReadFileEx(process->LocalPipe, rpstate->Buffer, FCGI_HEADER_LEN,
			&rpstate->Overlapped, &FcpReadPipeComplete))
		{
			goto Error;
		}
	} else {
		
		// We have not received the packet completely
		// Reallocate the buffer and begin another read operation
		if (RtlReallocateHeap(&rpstate, sizeof(FcpReadPipeState) + contentSize)) {
			goto Error;
		}
		
		memset(&rpstate->Overlapped, 0, sizeof(OVERLAPPED));
		if (!ReadFileEx(process->LocalPipe, &rpstate->Buffer[rpstate->Length],
			FCGI_HEADER_LEN + contentSize - rpstate->Length,
			&rpstate->Overlapped, &FcpReadPipeComplete))
		{
			goto Error;
		}
	}
	
	return;
Error:
	RtlFreeHeap(rpstate);
	FcpTerminateProcess(process, 1);
	ObDereferenceObject(process);
}

static void FcpWriteStdoutCompletion(void *state, size_t size, int error)
{
	RtlFreeHeap(state);
}

static void FcpWriteStdoutEofCompletion(void *state, size_t size, int error)
{
}