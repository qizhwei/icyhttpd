#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "daemon.h"

static RADIX_TREE *DmHandlerMapping;

CSTATUS DmInitServer(void)
{
	return RadixTreeCreate(&DmHandlerMapping);
}

CSTATUS DmCreateEndpoint(
	OUT DM_ENDPOINT **Endpoint,
	const char *IPAddress,
	int Port)
{
	DM_ENDPOINT *object;
	CSTATUS status;

	object = (DM_ENDPOINT *)malloc(sizeof(*object));
	if (!object)
		return C_BAD_ALLOC;

	status = RadixTreeCreate(&object->HostToNodeMapping);
	if (!SUCCESS(status)) {
		free(object);
		return status;
	}

	status = HttpInitEndpoint(&object->HttpEndpoint,
		IPAddress, Port);
	if (!SUCCESS(status)) {
		RadixTreeDestroy(object->HostToNodeMapping);
		free(object);
		return status;
	}

	object->DefaultNode = NULL;
	*Endpoint = object;
	return C_SUCCESS;
}

CSTATUS DmCreateNode(
	OUT DM_NODE **Node)
{
	DM_NODE *object;
	CSTATUS status;

	object = (DM_NODE *)malloc(sizeof(*object));
	if (!object)
		return C_BAD_ALLOC;

	status = RadixTreeCreate(&object->VirtualNodeMapping);
	if (!SUCCESS(status)) {
		free(object);
		return status;
	}

	status = RadixTreeCreate(&object->ExtensionToHandlerMapping);
	if (!SUCCESS(status)) {
		RadixTreeDestroy(object->VirtualNodeMapping);
		free(object);
		return status;
	}

	object->DefaultHandler = NULL;
	object->DocumentRoot = NULL;
	object->DocumentRootSizeInChars = 0;

	*Node = object;
	return C_SUCCESS;
}

void DmDestroyNode(
	DM_NODE *Node)
{
	RadixTreeDestroy(Node->ExtensionToHandlerMapping);
	RadixTreeDestroy(Node->VirtualNodeMapping);
	free(Node);
}

CSTATUS DmAttachNodeEndpoint(
	DM_ENDPOINT *Endpoint,
	DM_NODE *Node,
	OPT const char *HostName)
{
	CSTATUS status;

	if (!HostName) {
		if (Endpoint->DefaultNode)
			return C_ALREADY_EXISTS;

		Endpoint->DefaultNode = Node;
	} else {
		status = RadixTreeInsert(Endpoint->HostToNodeMapping, HostName, Node, 1);
		if (!SUCCESS(status))
			return status;
	}

	return C_SUCCESS;
}

CSTATUS DmAddVirtualNode(
	DM_NODE *Node,
	DM_NODE *SubNode,
	const char *Name)
{
	return RadixTreeInsert(Node->VirtualNodeMapping, Name, SubNode, 0);
}

CSTATUS DmSetDocumentRootNode(
	DM_NODE *Node,
	const wchar_t *DocumentRoot)
{
	wchar_t *dup = _wcsdup(DocumentRoot);
	if (!dup)
		return C_BAD_ALLOC;
	Node->DocumentRoot = dup;
	Node->DocumentRootSizeInChars = wcslen(dup);
	return C_SUCCESS;
}

CSTATUS DmAttachHandlerNode(
	DM_NODE *Node,
	DM_HANDLER *Handler,
	OPT const char *Extension)
{
	CSTATUS status;

	if (!Extension) {
		if (Node->DefaultHandler)
			return C_ALREADY_EXISTS;
		Node->DefaultHandler = Handler;
	} else {
		status = RadixTreeInsert(Node->ExtensionToHandlerMapping, Extension, Handler, 1);
		if (!SUCCESS(status))
			return status;
	}

	return C_SUCCESS;
}

void HttpMain(
	HTTP_CONNECTION *Connection,
	HTTP_REQUEST *Request,
	HTTP_RESPONSE *Response)
{
	DM_ENDPOINT *ep = (DM_ENDPOINT *)Connection->Endpoint;
	DM_NODE *node = NULL;
	DM_NODE *subNode;
	DM_HANDLER *handler = NULL;
	char *host = HttpGetHostRequest(Request);
	char *uri = HttpGetURIRequest(Request);
	char *p;
	char *ext;

	LogPrintf("HttpMain() is called.\n");

	if (*uri++ == '/') {
		if (host)
			node = (DM_NODE *)RadixTreeQueryCaseInsensitive(ep->HostToNodeMapping, host);
		if (!node)
			node = ep->DefaultNode;
	}

	if (!node) {
		HttpPrintfErrorResponse(Response, 403, "Forbidden", "<p>Node not found</p>");
		return;
	}

	// Follow URI to the last virtual node
	while (1) {
		p = strchr(uri, '/');
		if (p)
			*p = '\0';
		subNode = (DM_NODE *)RadixTreeQuery(node->VirtualNodeMapping, uri);
		if (p)
			*p = '/';

		if (!subNode)
			break;

		node = subNode;
		uri = p + 1;
	}

	// Get handler by extension or default
	ext = HttpGetExtensionRequest(Request);
	if (ext)
		handler = (DM_HANDLER *)RadixTreeQueryCaseInsensitive(
			node->ExtensionToHandlerMapping, ext);
	if (!handler)
		handler = node->DefaultHandler;

	if (!handler) {
		HttpPrintfErrorResponse(Response, 403, "Forbidden", "<p>Handler not found</p>");
		return;
	}

	handler->Type->InvokeFunc(handler, node, uri, Connection, Request, Response);
}

CSTATUS DmRegisterHandler(
	DM_HANDLER_TYPE *Type)
{
	CSTATUS status;

	if (Type->InitFunc) {
		status = Type->InitFunc();
		if (!SUCCESS(status))
			return status;
	}

	return RadixTreeInsert(DmHandlerMapping, Type->Name, Type, 1);
}

CSTATUS DmCreateHandler(
	OUT DM_HANDLER **Handler,
	const char *TypeName,
	const char *Param)
{
	DM_HANDLER_TYPE *type = (DM_HANDLER_TYPE *)
		RadixTreeQueryCaseInsensitive(DmHandlerMapping, TypeName);

	if (!type)
		return C_INVALID_NAME;

	return type->CreateFunc(Handler, type, Param);
}
