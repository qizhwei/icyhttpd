#ifndef HTTPD_DEAMON_H
#define HTTPD_DEAMON_H

#include <httpd_protocol.h>
#include <httpd_util.h>
#include <httpd_io.h>

typedef struct _DM_HANDLER_TYPE DM_HANDLER_TYPE;
typedef struct _DM_HANDLER DM_HANDLER;
typedef struct _DM_NODE DM_NODE;
typedef struct _DM_ENDPOINT DM_ENDPOINT;

typedef CSTATUS DM_HANDLER_INIT_FUNC(void);

typedef CSTATUS DM_HANDLER_CREATE_FUNC(
	OUT DM_HANDLER **Handler,
	DM_HANDLER_TYPE *Type,
	const char *Param);

typedef void DM_HANDLER_INVOKE_FUNC(
	DM_HANDLER *Handler,
	DM_NODE *Node,
	const char *RelativePath,
	HTTP_CONNECTION *Connection,
	HTTP_REQUEST *Request,
	HTTP_RESPONSE *Response);

struct _DM_HANDLER_TYPE {
	const char *Name;
	DM_HANDLER_INIT_FUNC *InitFunc;
	DM_HANDLER_CREATE_FUNC *CreateFunc;
	DM_HANDLER_INVOKE_FUNC *InvokeFunc;
};

struct _DM_HANDLER {
	DM_HANDLER_TYPE *Type;
};

struct _DM_NODE {
	RADIX_TREE *VirtualNodeMapping;
	RADIX_TREE *ExtensionToHandlerMapping;
	DM_HANDLER *DefaultHandler;
	const wchar_t *DocumentRoot;
	size_t DocumentRootSizeInChars;
};

struct _DM_ENDPOINT {
	HTTP_ENDPOINT HttpEndpoint;
	RADIX_TREE *HostToNodeMapping;
	DM_NODE *DefaultNode;
};

extern CSTATUS DmCreateEndpoint(
	OUT DM_ENDPOINT **Endpoint,
	const char *IPAddress,
	int Port);

extern CSTATUS DmCreateNode(
	OUT DM_NODE **Node);

extern void DmDestroyNode(
	DM_NODE *Node);

extern CSTATUS DmAttachNodeEndpoint(
	DM_ENDPOINT *Endpoint,
	DM_NODE *Node,
	OPT const char *HostName);

extern CSTATUS DmAddVirtualNode(
	DM_NODE *Node,
	DM_NODE *SubNode,
	const char *Name);

extern CSTATUS DmSetDocumentRootNode(
	DM_NODE *Node,
	const wchar_t *DocumentRoot);

extern CSTATUS DmAttachHandlerNode(
	DM_NODE *Node,
	DM_HANDLER *Handler,
	OPT const char *Extension);

extern CSTATUS DmRegisterHandler(
	DM_HANDLER_TYPE *Type);

extern CSTATUS DmCreateHandler(
	OUT DM_HANDLER **Handler,
	const char *TypeName,
	const char *Param);

#endif
