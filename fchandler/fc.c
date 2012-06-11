#include <assert.h>
#include <fcpool.h>
#include <httpd_daemon.h>

static CSTATUS FcHandlerInit(void)
{
	fc_startup();
	return C_SUCCESS;
}

static CSTATUS FcHandlerCreate(
	OUT DM_HANDLER **Handler,
	DM_HANDLER_TYPE *Type,
	const char *Param)
{
	// TODO
	return C_NOT_IMPLEMENTED;
}

static void FcHandlerInvoke(
	DM_HANDLER *Handler,
	DM_NODE *Node,
	const char *RelativePath,
	HTTP_CONNECTION *Connection,
	HTTP_REQUEST *Request,
	HTTP_RESPONSE *Response)
{
	// TODO
	assert(!"not implemented");
}

DM_HANDLER_TYPE FcHandlerType = {
	"fastcgi",
	FcHandlerInit,
	FcHandlerCreate,
	FcHandlerInvoke,
};
