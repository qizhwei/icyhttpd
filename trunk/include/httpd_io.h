#ifndef HTTPD_IO_H
#define HTTPD_IO_H

#include <stddef.h>
#include <stdint.h>
#include <cstatus.h>
#include <misc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _IO_THREAD IO_THREAD;
typedef void IO_THREAD_ENTRY(void *Context);
typedef struct _IO_LISTENER IO_LISTENER;
typedef struct _IO_CLIENT IO_CLIENT;
typedef struct _IO_FILE IO_FILE;

extern CSTATUS IoInitSystem(void);
extern void IoUninitSystem(void);
extern CSTATUS IoMainLoop(void);

extern CSTATUS IoCreateThread(
	OUT IO_THREAD **Thread,
	IO_THREAD_ENTRY *ThreadEntry,
	void *Context);

extern void IoDetachThread(
	IO_THREAD *Thread);

extern CSTATUS IoJoinThread(
	IO_THREAD *Thread);

extern CSTATUS IoCreateListener(
	OUT IO_LISTENER **Listener,
	const char *IPAddress,
	int Port);

extern void IoDestroyListener(
	IO_LISTENER *Listener);

extern CSTATUS IoCreateClientByAccept(
	OUT IO_CLIENT **Client,
	IO_LISTENER *Listener);

extern CSTATUS IoCreateClientByConnect(
	OUT IO_CLIENT **Client,
	const char *IPAddress,
	int Port);

extern void IoDestroyClient(
	IO_CLIENT *Client);

extern CSTATUS IoCreateFile(
	OUT IO_FILE **File,
	const wchar_t *Path);

extern CSTATUS IoGetSizeFile(
	IO_FILE *File,
	OUT uint64_t *FileSize);

extern void IoDestroyFile(
	IO_FILE *File);

extern CSTATUS IoReadClient(
	IO_CLIENT *Client,
	char *Buffer,
	size_t Size,
	OUT size_t *ActualSize);

extern CSTATUS IoWriteClient(
	IO_CLIENT *Client,
	const char *Buffer,
	size_t Size,
	OUT size_t *ActualSize);

extern CSTATUS IoTransmitFileClient(
	IO_CLIENT *Client,
	IO_FILE *File,
	uint64_t Offset,
	uint64_t Length);

#ifdef __cplusplus
}
#endif

#endif
