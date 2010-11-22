#include "fcp.h"
#include "rtl.h"
#include <assert.h>

typedef struct _FcpDispatchProcessState {
	OVERLAPPED Overlapped;
	FcpProcess *Process;
	size_t Length;
	unsigned char Buffer[FCGI_HEADER_LEN];
} FcpDispatchProcessState;

static void CALLBACK FcpDispatchProcessComplete(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped);
static void FcpWriteStdoutCompletion(void *state, size_t size, int error);
static void FcpWriteStdoutEofCompletion(void *state, size_t size, int error);

int FcpDispatchProcess(FcpProcess *process)
{
	FcpDispatchProcessState *dpstate;
	
	// Allocate memory for asynchronous state object
	dpstate = RtlAllocateHeap(sizeof(FcpDispatchProcessState));
	if (dpstate == NULL) {
		return 1;
	}
	
	// Initialize the state object
	memset(&dpstate->Overlapped, 0, sizeof(OVERLAPPED));
	dpstate->Process = ObReferenceObjectByPointer(process, NULL);
	dpstate->Length = 0;
	
	// Begin reading from the process pipe
	if (!ReadFileEx(process->LocalPipe, dpstate->Buffer, FCGI_HEADER_LEN,
		&dpstate->Overlapped, &FcpDispatchProcessComplete))
	{
		RtlFreeHeap(dpstate);
		ObDereferenceObject(process);
		return 1;
	}
	
	return 0;
}

static void CALLBACK FcpDispatchProcessComplete(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped)
{
	FcpDispatchProcessState *dpstate = (FcpDispatchProcessState *)overlapped;
	FcpProcess *process = dpstate->Process;
	FcRequest *request;
	FCGI_Header *header;
	size_t contentLength, contentSize;
	
	if (errorCode != 0) {
		RtlFreeHeap(dpstate);
		FcpTerminateProcess(process, 1);
		ObDereferenceObject(process);
		return;
	}
	
	dpstate->Length += bytesTransferred;
	
	if (dpstate->Length >= FCGI_HEADER_LEN) {
		header = (FCGI_Header *)&dpstate->Buffer[0];
		contentLength = (header->contentLengthB1 << 8) + header->contentLengthB0;
		contentSize = contentLength + header->paddingLength;
	} else {
		contentSize = 0;
	}
	
	assert(dpstate->Length <= FCGI_HEADER_LEN + contentSize);
	
	if (dpstate->Length == FCGI_HEADER_LEN + contentSize) {
		
		// We have received a complete packet
		if (process->State != FCP_STATE_INTERACTIVE) {
			goto Error;
		}
		
		request = process->Request;
		
		switch (header->type) {
			case FCGI_STDOUT:
				if (RtlWriteFifo(request->StdoutFifo, &dpstate->Buffer[FCGI_HEADER_LEN],
					 contentLength, &FcpWriteStdoutCompletion, dpstate))
				{
					goto Error;
				}
				
				// Since the writing operation has borrowed the state, we must create a new one
				dpstate = NULL;
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
		if (RtlReallocateHeap(&dpstate, sizeof(FcpDispatchProcessState) + contentSize)) {
			goto Error;
		}
		
		// Since the dpstate might be a brand new buffer, we must reinitialize it all over
		// Leave the reference count of the process unchanged
		memset(&dpstate->Overlapped, 0, sizeof(OVERLAPPED));
		dpstate->Process = process;
		dpstate->Length = 0;
		
		// Begin read operation
		if (!ReadFileEx(process->LocalPipe, dpstate->Buffer, FCGI_HEADER_LEN,
			&dpstate->Overlapped, &FcpDispatchProcessComplete))
		{
			goto Error;
		}
	} else {
		
		// We have not received the packet completely
		// Reallocate the buffer and begin another read operation
		if (RtlReallocateHeap(&dpstate, sizeof(FcpDispatchProcessState) + contentSize)) {
			goto Error;
		}
		
		memset(&dpstate->Overlapped, 0, sizeof(OVERLAPPED));
		if (!ReadFileEx(process->LocalPipe, &dpstate->Buffer[dpstate->Length],
			FCGI_HEADER_LEN + contentSize - dpstate->Length,
			&dpstate->Overlapped, &FcpDispatchProcessComplete))
		{
			goto Error;
		}
	}
	
	return;
Error:
	RtlFreeHeap(dpstate);
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