#include "fcp.h"
#include <assert.h>

typedef struct _FcpBeginRequestState {
	FcRequest *Request;
	int PendingIos;
	int Error;
	void *Buffer;
	FcBeginRequestCompletion *Completion;
	void *CompletionState;
} FcpBeginRequestState;

static const FCGI_BeginRequestRecord FcpBeginRequestPacket = {
	{ FCGI_VERSION_1, FCGI_BEGIN_REQUEST, 0, 0, 0, 8, },
	{ 0, FCGI_RESPONDER, FCGI_KEEP_CONN, },
};

static const FCGI_Header FcpParamsPacketInitialHeader = {
	FCGI_VERSION_1, FCGI_PARAMS,
};

static void FcpBeginTrigger(void *state, size_t size, int error)
{
	FcpBeginRequestState *brstate = (FcpBeginRequestState *)state;
	FcRequest *request;
	
	if (error)
		brstate->Error = 1;
	
	assert(brstate->PendingIos > 0);
	brstate->PendingIos -= 1;
	
	if (brstate->PendingIos == 0) {
		request = brstate->Request;
		if (brstate->Error) {
			FcpTerminateProcess(request->Process, 1);
			FcpDereferenceRequest(request);
			brstate->Completion(brstate->CompletionState, NULL, 1);
		} else {
			brstate->Completion(brstate->CompletionState, request, 0);
		}
		RtlFreeHeap(brstate);
	}
}

static void FcpSendParamComplete(void *state, size_t size, int error)
{
	void *buffer = (unsigned char *)state - sizeof(FcpBeginRequestState *);
	FcpBeginRequestState *brstate = *(FcpBeginRequestState **)buffer;
	RtlFreeHeap(buffer);
	FcpBeginTrigger(brstate, size, error);
}

static void FcpSendParam(FcpBeginRequestState *brstate, const char *key, const char *value)
{
	FcRequest *request = brstate->Request;
	FcProcess *process = request->Process;
	unsigned char *buffer = brstate->Buffer;
	FCGI_Header *header;
	size_t oldLength, oldSize;
	size_t keyLength, keySize;
	size_t valueLength, valueSize;
	size_t newLength, newSize;
	size_t offset;
	
	// Allocate buffer if we don't have one
	if (buffer == NULL) {
		buffer = RtlAllocateHeap(sizeof(FcpBeginRequestState *) + sizeof(FCGI_Header), "FcpSendParam");
		if (buffer == NULL) {
			brstate->PendingIos += 1;
			FcpBeginTrigger(brstate, 0, 1);
			return;
		}
		
		*(FcpBeginRequestState **)buffer = brstate;
		buffer += sizeof(FcpBeginRequestState *);
		brstate->Buffer = buffer;
		header = (FCGI_Header *)buffer;
		*header = FcpParamsPacketInitialHeader;
	} else {
		header = (FCGI_Header *)buffer;
	}
	
	oldLength = (header->contentLengthB1 << 8) + header->contentLengthB0;
	oldSize = oldLength + sizeof(FCGI_Header);
	
	if (key == NULL) {
		brstate->PendingIos += 1;
		if (FcpWriteProcess(process, buffer, oldSize, &FcpSendParamComplete, buffer)) {
			RtlFreeHeap(buffer - sizeof(FcpBeginRequestState *));
			FcpBeginTrigger(brstate, 0, 1);
			return;
		}
		brstate->Buffer = NULL;
		
		// We should always end by an empty packet
		if (oldLength != 0) {
			FcpSendParam(brstate, key, value);
		}
		
		return;
	}
	
	keyLength = strlen(key);
	keySize = keyLength + (keyLength < 0x80 ? 1 : 4);
	valueLength = strlen(value);
	valueSize = valueLength + (valueLength < 0x80 ? 1 : 4);
	newLength = oldLength + keySize + valueSize;
	newSize = newLength + sizeof(FCGI_Header);
	
	if (newSize >= 0x10000) {
		
		// The new packet size would exceed 64KB therefore
		// we must send it and start a new packet
		brstate->PendingIos += 1;
		if (FcpWriteProcess(process, buffer, oldSize, &FcpSendParamComplete, buffer)) {
			RtlFreeHeap(buffer - sizeof(FcpBeginRequestState *));
			FcpBeginTrigger(brstate, 0, 1);
			return;
		}
		brstate->Buffer = NULL;
		return FcpSendParam(brstate, key, value);
	}
	
	// Reallocate the buffer to neccessary size
	buffer -= sizeof(FcpBeginRequestState *);
	if (RtlReallocateHeap(&buffer, sizeof(FcpBeginRequestState *) + newSize, "FcpSendParam")) {
		RtlFreeHeap(buffer);
		brstate->PendingIos += 1;
		FcpBeginTrigger(brstate, 0, 1);
		return;
	}
	buffer += sizeof(FcpBeginRequestState *);
	
	// Update all references to the buffer
	header = (FCGI_Header *)buffer;
	brstate->Buffer = buffer;
	
	// Update the content length
	header->contentLengthB1 = newLength >> 8;
	header->contentLengthB0 = newLength;
	
	// Write key and value onto the buffer
	offset = oldSize;
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

	memcpy(&buffer[offset], key, keyLength);
	memcpy(&buffer[offset + keyLength], value, valueLength);
}

int FcBeginRequest(FcPool *pool, const char *scriptPath, FcBeginRequestCompletion *completion, void *state)
{
	FcpBeginRequestState *brstate;
	FcProcess *process;
	FcRequest *request;

	// Allocate memory for request and asynchronous state
	request = RtlAllocateHeap(sizeof(FcRequest), "FcBeginRequest.request");
	if (request == NULL) {
		return 1;
	}
	
	brstate = RtlAllocateHeap(sizeof(FcpBeginRequestState), "FcBeginRequest.brstate");
	if (brstate == NULL) {
		RtlFreeHeap(request);
		return 1;
	}
	
	// Get an process instance from the pool or by creation
	process = FcpPopPoolProcess(pool);
	if (process == NULL) {
		process = FcpCreateProcess(pool);
		if (process == NULL) {
			RtlFreeHeap(brstate);
			RtlFreeHeap(request);
			return 1;
		}
	}
	
	// Create fifo for stdin and stdout
	request->StdinFifo = RtlCreateFifo();
	if (request->StdinFifo == NULL) {
		if (FcpPushPoolProcess(process)) {
			FcpTerminateProcess(process, 1);
		}
		RtlFreeHeap(brstate);
		RtlFreeHeap(request);
		return 1;
	}
	
	request->StdoutFifo = RtlCreateFifo();
	if (request->StdoutFifo == NULL) {
		if (FcpPushPoolProcess(process)) {
			FcpTerminateProcess(process, 1);
		}
		RtlDestroyFifo(request->StdinFifo);
		RtlFreeHeap(brstate);
		RtlFreeHeap(request);
		return 1;
	}
	
	// Add a reference to the process
	process->ReferenceCount += 1;

	// Initialize request object and asynchronous state
	request->ReferenceCount = 2;
	request->Process = process;
	brstate->Request = request;
	brstate->PendingIos = 1;
	brstate->Error = 0;
	brstate->Buffer = NULL;
	brstate->Completion = completion;
	brstate->CompletionState = state;
	
	process->Request = request;
	process->State = FCP_STATE_INTERACTIVE;
	
	// Write the begin request packet
	if (FcpWriteProcess(process, &FcpBeginRequestPacket, sizeof(FcpBeginRequestPacket), &FcpBeginTrigger, brstate)) {
		FcpTerminateProcess(process, 1);
		FcpDereferenceRequest(request);
		RtlFreeHeap(brstate);
		return 1;
	}
	
	// Send parameters
	FcpSendParam(brstate, "PATH_TRANSLATED", scriptPath);
	FcpSendParam(brstate, NULL, NULL);
		
	return 0;
}

void FcpDereferenceRequest(FcRequest *request)
{
	assert(request->ReferenceCount > 0);

	request->ReferenceCount -= 1;
	if (request->ReferenceCount == 0) {
		FcpDereferenceProcess(request->Process);
		RtlDestroyFifo(request->StdinFifo);
		RtlDestroyFifo(request->StdoutFifo);
		RtlFreeHeap(request);
	}
}

typedef struct _FcpRequestIoState {
	FcRequest *Request;
	FcReadWriteCompletion *Completion;
	void *CompletionState;
} FcpRequestIoState;

static void FcpRequestIoComplete(void *state, size_t size, int error)
{
	FcpRequestIoState *ristate = state;
	ristate->Completion(ristate->CompletionState, size, error);
	FcpDereferenceRequest(ristate->Request);
	RtlFreeHeap(ristate);
}

int FcReadRequest(FcRequest *request, char *buffer, size_t size, FcReadWriteCompletion *completion, void *state)
{
	FcpRequestIoState *ristate = RtlAllocateHeap(sizeof(FcpRequestIoState), "FcReadRequest");
	
	if (ristate == NULL) {
		return 1;
	}
	
	request->ReferenceCount += 1;
	ristate->Request = request;
	ristate->Completion = completion;
	ristate->CompletionState = state;
	
	return RtlReadFifo(request->StdoutFifo, buffer, size, FcpRequestIoComplete, ristate);
}

int FcWriteRequest(FcRequest *request, char *buffer, size_t size, FcReadWriteCompletion *completion, void *state)
{
	FcpRequestIoState *ristate = RtlAllocateHeap(sizeof(FcpRequestIoState), "FcWriteRequest");
	
	if (ristate == NULL) {
		return 1;
	}
	
	request->ReferenceCount += 1;
	ristate->Request = request;
	ristate->Completion = completion;
	ristate->CompletionState = state;

	return RtlWriteFifo(request->StdinFifo, buffer, size, FcpRequestIoComplete, ristate);
}

void FcEndRequest(FcRequest *request)
{
	FcpDereferenceRequest(request);
}
