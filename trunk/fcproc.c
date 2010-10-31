#include "fcp.h"
#include <string.h>
#include <assert.h>

FcProcess * FcpCreateProcess(FcPool *pool)
{
	HANDLE localPipe, remotePipe;
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	FcProcess *process;
	
	// Create a pair of pipe
	if (RtlCreatePipe(&remotePipe, &localPipe)) {
		return NULL;
	}
	
	// Allow the server pipe to be inherited to child process
	if (!SetHandleInformation(remotePipe, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
		CloseHandle(localPipe);
		CloseHandle(remotePipe);
		return NULL;
	}
	
	// Initialize startup info for process creation
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = remotePipe;
	si.hStdOutput = INVALID_HANDLE_VALUE;
	si.hStdError = INVALID_HANDLE_VALUE;
	
	// Create child process
	if (!CreateProcessA(NULL, pool->CommandLine, NULL, NULL, TRUE,
		CREATE_BREAKAWAY_FROM_JOB | CREATE_SUSPENDED, NULL, NULL, &si, &pi))
	{
		CloseHandle(localPipe);
		CloseHandle(remotePipe);
		return NULL;
	}
	
	// Close the remote end of the pipe since it had already been
	// duplicated to the child process and we no longer need it
	CloseHandle(remotePipe);
	
	// Assign the process to job object
	if (!AssignProcessToJobObject(FcpJobObject, pi.hProcess)) {
		TerminateProcess(pi.hProcess, 1);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(localPipe);
		return NULL;
	}
	
	// The process has been assigned to a job object
	// we can therefore safely let its initial thread run
	if (!ResumeThread(pi.hThread)) {
		TerminateProcess(pi.hProcess, 1);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(localPipe);
		return NULL;
	}
	
	// Close the thread handle
	CloseHandle(pi.hThread);
	
	// Allocate memory for the object
	process = RtlAllocateHeap(sizeof(FcProcess), "FcpCreateProcess");
	if (process == NULL) {
		TerminateProcess(pi.hProcess, 1);
		CloseHandle(pi.hProcess);
		CloseHandle(localPipe);
		return NULL;
	}
	
	// Increment reference count of the pool
	pool->ReferenceCount += 1;
	
	// Initialize the object
	process->ReferenceCount = 0;
	process->RemainingRequests = pool->MaxRequests;
	process->Pool = pool;
	InsertHeadList(&pool->RunningList, &process->PoolEntry);
	process->State = FCP_STATE_READY;
	process->ProcessHandle = pi.hProcess;
	process->LocalPipe = localPipe;
	process->Request = NULL;
	process->WaitBlock = NULL;
	
	// Start reading the process
	if (FcpReadPipe(process)) {
		FcpTerminateProcess(process, 1);
		return NULL;
	}
	
	return process;
}

typedef struct _FcpWriteProcessState {
	OVERLAPPED Overlapped;
	FcReadWriteCompletion *Completion;
	void *CompletionState;
} FcpWriteProcessState;

static void CALLBACK FcpWriteProcessComplete(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped)
{
	FcpWriteProcessState *wpstate = (FcpWriteProcessState *)overlapped;
	wpstate->Completion(wpstate->CompletionState, bytesTransferred, errorCode != 0);
	RtlFreeHeap(wpstate);
}

int FcpWriteProcess(FcProcess *process, const void *buffer, size_t size, FcReadWriteCompletion *completion, void *state)
{
	FcpWriteProcessState *wpstate;
	
	wpstate = RtlAllocateHeap(sizeof(FcpWriteProcessState), "FcpWriteProcess");
	if (wpstate == NULL) {
		return 1;
	}
	
	memset(&wpstate->Overlapped, 0, sizeof(OVERLAPPED));
	wpstate->Completion = completion;
	wpstate->CompletionState = state;
	
	if (!WriteFileEx(process->LocalPipe, buffer, size, &wpstate->Overlapped, &FcpWriteProcessComplete)) {
		RtlFreeHeap(wpstate);
		return 1;
	}
	
	return 0;
}

static void FcpWriteEofComplete(void *state, size_t size, int error)
{
}

int FcpTerminateProcess(FcProcess *process, int error)
{
	if (process->State == FCP_STATE_POOLING) {
		FcpRemovePoolProcess(process);
	} else if (process->State == FCP_STATE_INTERACTIVE) {
		RtlWriteFifo(process->Request->StdoutFifo, NULL, 0, FcpWriteEofComplete, NULL);
	} else if (process->State == FCP_STATE_TERMINATED) {
		return 1;
	}
	
	if (!TerminateProcess(process->ProcessHandle, error)) {
		return 1;
	}
	
	RemoveEntryList(&process->PoolEntry);
	process->State = FCP_STATE_TERMINATED;
	return 0;
}

void FcpDereferenceProcess(FcProcess *process)
{
	assert(process->ReferenceCount > 0);
	
	// Decrement reference count
	process->ReferenceCount -= 1;
	
	// If there is no further reference, free all resources
	if (process->ReferenceCount == 0) {
		FcpTerminateProcess(process, 0);
		FcpDereferencePool(process->Pool);
		CloseHandle(process->ProcessHandle);
		CloseHandle(process->LocalPipe);
		RtlFreeHeap(process);
	}
}
