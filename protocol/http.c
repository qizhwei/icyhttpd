#include <stdlib.h>
#include <httpd_protocol.h>
#include <httpd_io.h>
#include <httpd_util.h>
#include "protocol.h"

static void HttpClientThreadEntry(
	void *Context)
{
	HTTP_CONNECTION *conn = (HTTP_CONNECTION *)Context;
	BUFFERED_READER reader;
	HTTP_REQUEST request;
	HTTP_RESPONSE response;
	CSTATUS status;
	int keepAlive;

	status = BufferedReaderInit(&reader, BUFFER_INITIAL_ALLOC_SIZE,
		(READ_FUNCTION *)IoReadClient, conn->Client);
	if (!SUCCESS(status)) {
		IoDestroyClient(conn->Client);
		free(conn);
		return;
	}

	do {
		status = HttpInitResponse(&response, conn->Client);
		if (!SUCCESS(status)) {
			BufferedReaderUninit(&reader);
			IoDestroyClient(conn->Client);
			free(conn);
			return;
		}

		status = HttpInitRequest(&request, &reader);
		if (SUCCESS(status)) {
			HttpAttachRequestResponse(&response, &request);
			HttpMain(conn, &request, &response);
			HttpUninitRequest(&request);
		} else {
			HttpWriteErrorStatusResponse(&response, status);
		}

		keepAlive = response.KeepAlive;
		HttpUninitResponse(&response);
	} while (keepAlive);

	BufferedReaderUninit(&reader);
	IoDestroyClient(conn->Client);
	free(conn);
}

static void HttpListenerThreadEntry(
	void *Context)
{
	HTTP_ENDPOINT *ep = (HTTP_ENDPOINT *)Context;
	HTTP_CONNECTION *conn;
	IO_THREAD *thread;
	CSTATUS status;

	while (1) {
		conn = (HTTP_CONNECTION *)malloc(sizeof(HTTP_CONNECTION));
		if (!conn) {
			LogPrintf("malloc() failed\n");
			continue;
		}

		conn->Endpoint = ep;
		status = IoCreateClientByAccept(&conn->Client, ep->Listener);
		if (!SUCCESS(status)) {
			free(conn);
			LogPrintf("IoCreateClientByAccept() failed with %d\n", (int)status);
			continue;
		}

		status = IoCreateThread(&thread, HttpClientThreadEntry, conn);
		if (!SUCCESS(status)) {
			free(conn);
			IoDestroyClient(conn->Client);
			LogPrintf("IoCreateThread() failed with %d\n", (int)status);
			continue;
		}

		IoDetachThread(thread);
	}
}

CSTATUS HttpInitEndpoint(
	OUT HTTP_ENDPOINT *Endpoint,
	const char *IPAddress,
	int Port)
{
	CSTATUS status;

	status = IoCreateListener(&Endpoint->Listener, IPAddress, Port);
	if (!SUCCESS(status))
		return status;

	status = IoCreateThread(&Endpoint->Thread, HttpListenerThreadEntry, Endpoint);
	if (!SUCCESS(status)) {
		IoDestroyListener(Endpoint->Listener);
		return status;
	}

	return C_SUCCESS;
}
