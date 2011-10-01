#include "fcp.h"
#include <assert.h>

typedef struct _FcpBeginRequestState {
	FcRequest *Request;
	int PendingIos;
	int Error;
	void *Buffer;
} FcpBeginRequestState;

static const FCGI_BeginRequestRecord FcpBeginRequestPacket = {
	{ FCGI_VERSION_1, FCGI_BEGIN_REQUEST, 0, 0, 0, 8, },
	{ 0, FCGI_RESPONDER, FCGI_KEEP_CONN, },
};

static const FCGI_Header FcpParamsPacketInitialHeader = {
	FCGI_VERSION_1, FCGI_PARAMS,
};

static const FCGI_Header FcpStdinPacketInitialHeader = {
	FCGI_VERSION_1, FCGI_STDIN,
};

static void FcpBeginTrigger(void *state, size_t size, int error)
{
	FcpBeginRequestState *brstate = (FcpBeginRequestState *)state;
	FcRequest *request;
	
	if (error) {
		brstate->Error = 1;
	}
	
	assert(brstate->PendingIos > 0);
	brstate->PendingIos -= 1;
	
	if (brstate->PendingIos == 0) {
		request = brstate->Request;
		if (brstate->Error) {
			FcpTerminateProcess(request->Process, 1);
		}
		RtlFreeHeap(brstate);
		ObDereferenceObject(request);
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
	FcpProcess *process = request->Process;
	unsigned char *buffer = brstate->Buffer;
	FCGI_Header *header;
	size_t oldLength, oldSize;
	size_t keyLength, keySize;
	size_t valueLength, valueSize;
	size_t newLength, newSize;
	size_t offset;
	
	// Allocate buffer if we don't have one
	if (buffer == NULL) {
		buffer = RtlAllocateHeap(sizeof(FcpBeginRequestState *) + sizeof(FCGI_Header));
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
		FcpSendParam(brstate, key, value);
		return;
	}
	
	// Reallocate the buffer to neccessary size
	buffer -= sizeof(FcpBeginRequestState *);
	if (RtlReallocateHeap(&buffer, sizeof(FcpBeginRequestState *) + newSize)) {
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

#define FCP_BLOCK_COUNT (2)
#define FCP_BLOCK_STATE_FREE (0)
#define FCP_BLOCK_STATE_ACTIVE (1)
#define FCP_BLOCK_STATE_BUSY (2)
#define FCP_BUFFER_SIZE (4096)

typedef struct _FcpTransferBlock {
	char Index;
	char State;
	char Buffer[FCP_BUFFER_SIZE];
} FcpTransferBlock;

typedef struct _FcpTransferState {
	FcRequest *Request;
	int PendingIos;
	int Error;
	FcpTransferBlock Block[FCP_BLOCK_COUNT];
	char EofIndex;
	char EofState;
} FcpTransferState;

static void FcpTransferFifo(RtlFifo *fifo, FcpTransferState *tstate, RtlIoCompletion *completion);

static void FcpTransferStdinCompletion(void *state, size_t size, int error)
{
	FcpTransferBlock *block = state;
	FcpTransferState *tstate = CONTAINING_RECORD(block, FcpTransferState, Block[block->Index]);
	FcRequest *request = tstate->Request;
	FCGI_Header *header;
	
	if (error) {
		tstate->Error = 1;
	}
	
	assert(tstate->PendingIos >= 1);
	
	switch (block->State) {
		case FCP_BLOCK_STATE_ACTIVE:
		
			if (!error) {
				// Switch the state
				block->State = FCP_BLOCK_STATE_BUSY;
				
				// Fill in the header
				header = (FCGI_Header *)block->Buffer;
				*header = FcpStdinPacketInitialHeader;
				header->contentLengthB0 = size;
				header->contentLengthB1 = size >> 8;
				
				// Write to the process
				if (FcpWriteProcess(request->Process, block->Buffer, FCGI_HEADER_LEN + size, &FcpTransferStdinCompletion, block)) {
					FcpTransferStdinCompletion(block, 0, 1);
				}
			}

			if (size != FCP_BUFFER_SIZE - FCGI_HEADER_LEN || error) {
				
				// End the stream
				tstate->PendingIos += 1;
				tstate->EofState = FCP_BLOCK_STATE_BUSY;
				if (FcpWriteProcess(request->Process, &FcpStdinPacketInitialHeader, FCGI_HEADER_LEN, &FcpTransferStdinCompletion, &tstate->EofIndex)) {
					FcpTransferStdinCompletion(&tstate->EofIndex, 0, 1);
				}
				
				return;
			}
			
			break;
			
		case FCP_BLOCK_STATE_BUSY:
		
			// Switch the state
			block->State = FCP_BLOCK_STATE_FREE;
			
			// Decrement the reference count
			tstate->PendingIos -= 1;
			if (tstate->PendingIos == 0) {
				if (tstate->Error) {
					FcpTerminateProcess(request->Process, 1);
				}
				ObDereferenceObject(request);
				RtlFreeHeap(tstate);
				return;
			}
			
			break;
		default:
			assert(0);
	}
	
	// Start transferring another block if not EOF
	if (tstate->EofState == FCP_BLOCK_STATE_FREE) {
		FcpTransferFifo(request->StdinFifo, tstate, &FcpTransferStdinCompletion);
	}
}

static void FcpTransferFifo(RtlFifo *fifo, FcpTransferState *tstate, RtlIoCompletion *completion)
{
	int index;
	int free = -1;
	
	for (index = 0; index < FCP_BLOCK_COUNT; ++index) {
		
		switch (tstate->Block[index].State) {
			case FCP_BLOCK_STATE_FREE:
				if (free == -1) {
					free = index;
				}
				break;
			case FCP_BLOCK_STATE_ACTIVE:
				// There should be only one active block
				return;
		}
	}
	
	if (free == -1) {
		return;
	}
	
	tstate->PendingIos += 1;
	tstate->Block[free].State = FCP_BLOCK_STATE_ACTIVE;
	if (RtlReadFifo(fifo, &tstate->Block[free].Buffer[FCGI_HEADER_LEN],
		FCP_BUFFER_SIZE - FCGI_HEADER_LEN, completion, &tstate->Block[free]))
	{
		completion(&tstate->Block[free], 0, 1);
	}
}

FcRequest * FcBeginRequest(FcPool *pool, FcRequestParam *param)
{
	FcpBeginRequestState *brstate;
	FcpProcess *process;
	FcRequest *request;
	FcpTransferState *tstate;
	int index;

	// Allocate memory for request and asynchronous states
	request = ObCreateObject(&FcpRequestObjectType, sizeof(FcRequest), NULL, NULL);
	if (request == NULL) {
		return NULL;
	}
	
	brstate = RtlAllocateHeap(sizeof(FcpBeginRequestState));
	if (brstate == NULL) {
		ObDereferenceObject(request);
		return NULL;
	}
	
	tstate = RtlAllocateHeap(sizeof(FcpTransferState));
	if (tstate == NULL) {
		ObDereferenceObject(request);
		RtlFreeHeap(brstate);
	}
	
	// Get an process instance from the pool or by creation
	process = FcpPopPoolProcess(pool);
	if (process == NULL) {
		process = FcpCreateProcess(pool);
		if (process == NULL) {
			ObDereferenceObject(request);
			RtlFreeHeap(tstate);
			RtlFreeHeap(brstate);
			return NULL;
		}
	}
	
	// Create fifo for stdin and stdout
	request->StdinFifo = RtlCreateFifo();
	request->StdoutFifo = RtlCreateFifo();

	if (request->StdinFifo == NULL || request->StdoutFifo == NULL) {
		if (FcpPushPoolProcess(process)) {
			FcpTerminateProcess(process, 1);
			ObDereferenceObject(process);
		}
		ObDereferenceObject(request);
		RtlFreeHeap(tstate);
		RtlFreeHeap(brstate);
		return NULL;
	}
	
	// Initialize request object and asynchronous states
	request->Process = process;
	brstate->Request = ObReferenceObjectByPointer(request, NULL);
	brstate->PendingIos = 1;
	brstate->Error = 0;
	brstate->Buffer = NULL;
	tstate->Request = ObReferenceObjectByPointer(request, NULL);
	tstate->PendingIos = 0;
	tstate->Error = 0;
	
	for (index = 0; index < FCP_BLOCK_COUNT; ++index) {
		FcpTransferBlock *block = &tstate->Block[index];
		block->Index = index;
		block->State = FCP_BLOCK_STATE_FREE;
	}
	
	tstate->EofIndex = FCP_BLOCK_COUNT;
	tstate->EofState = FCP_BLOCK_STATE_FREE;
	
	process->Request = ObReferenceObjectByPointer(request, NULL);
	process->State = FCP_STATE_INTERACTIVE;
	
	// Write the begin request packet
	if (FcpWriteProcess(process, &FcpBeginRequestPacket, sizeof(FcpBeginRequestPacket), &FcpBeginTrigger, brstate)) {
		FcpTerminateProcess(process, 1);
		ObDereferenceObject(process);
		ObDereferenceObject(request);
		RtlFreeHeap(tstate);
		RtlFreeHeap(brstate);
		return NULL;
	}
	
	// Send parameters
	FcpSendParam(brstate, "SCRIPT_FILENAME", param->ScriptFilename);
	FcpSendParam(brstate, "REQUEST_METHOD", param->RequestMethod);
	FcpSendParam(brstate, "CONTENT_LENGTH", param->ContentLength);
	FcpSendParam(brstate, "CONTENT_TYPE", param->ContentType);
	FcpSendParam(brstate, NULL, NULL);
	
	// Transfer fifos
	FcpTransferFifo(request->StdinFifo, tstate, &FcpTransferStdinCompletion);
		
	return request;
}

void FcpRequestClose(void *object)
{
	FcRequest *request = object;
	
	if (request->Process != NULL) {
		ObDereferenceObject(request->Process);
	}
	
	if (request->StdinFifo != NULL) {
		RtlDestroyFifo(request->StdinFifo);
	}
	
	if (request->StdoutFifo != NULL) {
		RtlDestroyFifo(request->StdoutFifo);
	}
}

typedef struct _FcpRequestIoState {
	FcRequest *Request;
	RtlIoCompletion *Completion;
	void *CompletionState;
} FcpRequestIoState;

static void FcpRequestIoComplete(void *state, size_t size, int error)
{
	FcpRequestIoState *ristate = state;
	ristate->Completion(ristate->CompletionState, size, error);
	ObDereferenceObject(ristate->Request);
	RtlFreeHeap(ristate);
}

int FcReadRequest(FcRequest *request, char *buffer, size_t size, RtlIoCompletion *completion, void *state)
{
	FcpRequestIoState *ristate = RtlAllocateHeap(sizeof(FcpRequestIoState));
	
	if (ristate == NULL) {
		return 1;
	}
	
	ristate->Request = ObReferenceObjectByPointer(request, NULL);
	ristate->Completion = completion;
	ristate->CompletionState = state;
	
	return RtlReadFifo(request->StdoutFifo, buffer, size, FcpRequestIoComplete, ristate);
}

int FcWriteRequest(FcRequest *request, char *buffer, size_t size, RtlIoCompletion *completion, void *state)
{
	FcpRequestIoState *ristate = RtlAllocateHeap(sizeof(FcpRequestIoState));
	
	if (ristate == NULL) {
		return 1;
	}
	
	ristate->Request = ObReferenceObjectByPointer(request, NULL);
	ristate->Completion = completion;
	ristate->CompletionState = state;

	return RtlWriteFifo(request->StdinFifo, buffer, size, FcpRequestIoComplete, ristate);
}
