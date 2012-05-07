#include <stdlib.h>
#include <assert.h>
#include "naiveio.h"

static DWORD WINAPI NaiveIoThreadEntry(
	LPVOID lpParameter)
{
	IO_THREAD *Thread = (IO_THREAD *)lpParameter;
	WaitForSingleObject(IopRunEvent, INFINITE);
	Thread->ThreadEntry(Thread->UserContext);
	IoDetachThread(Thread);
	return 0;
}

CSTATUS IoCreateThread(
	OUT IO_THREAD **Thread,
	IO_THREAD_ENTRY *ThreadEntry,
	void *Context)
{
	IO_THREAD *object;
	CSTATUS status;

	object = (IO_THREAD *)malloc(sizeof(IO_THREAD));
	if (object == NULL) {
		return C_BAD_ALLOC;
	}

	// One for user, one for the thread's execution
	object->ReferenceCount = 2;

	object->ThreadEntry = ThreadEntry;
	object->UserContext = Context;

	object->ThreadHandle = CreateThread(
		NULL, 0, NaiveIoThreadEntry, object, 0, NULL);
	if (object->ThreadHandle == NULL) {
		status = Win32ErrorCodeToCStatus(GetLastError());
		free(object);
		return status;
	}

	*Thread = object;
	return C_SUCCESS;
}

void IoDetachThread(
	IO_THREAD *Thread)
{
	assert(Thread->ReferenceCount > 0);

	if (InterlockedDecrement(&Thread->ReferenceCount) == 0) {
		CloseHandle(Thread->ThreadHandle);
		free(Thread);
	}
}

CSTATUS IoJoinThread(
	IO_THREAD *Thread)
{
	DWORD WaitResult = WaitForSingleObject(Thread->ThreadHandle, INFINITE);

	// Detach the thread even if the wait is failed
	IoDetachThread(Thread);

	if (WaitResult == WAIT_FAILED) {
		return Win32ErrorCodeToCStatus(GetLastError());
	}

	return C_SUCCESS;
}
