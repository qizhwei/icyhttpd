#include "svp.h"
#include <assert.h>

static int SvpEqualEndpointIpv4(SvpEndpointIpv4 *endpoint0, SvpEndpointIpv4 *endpoint1)
{
	if (endpoint0->Address != endpoint1->Address) {
		return 1;
	}
	
	if (endpoint0->Port != endpoint1->Port) {
		return 1;
	}
	
	return 0;
}

static int SvpEqualEndpoint(SvpEndpoint *endpoint0, SvpEndpoint *endpoint1)
{
	switch (endpoint0->Type) {
		case SVP_ENDPOINT_IPV4:
			if (endpoint1->Type != SVP_ENDPOINT_IPV4) {
				return 1;
			}
			
			return SvpEqualEndpointIpv4((SvpEndpointIpv4 *)endpoint0, (SvpEndpointIpv4 *)endpoint1);
		default:
			assert(0);
	}
}

static SvpListener * SvpFindListener(SvpEndpoint *endpoint)
{
	ListEntry *entry;
	SvpListener *listener;
	
	for (entry = SvpListenerList.Flink; entry != &SvpListenerList; entry = entry->Flink) {
		listener = CONTAINING_RECORD(entry, SvpListener, Entry);
		if (!SvpEqualEndpoint(listener->Endpoint, endpoint)) {
			return listener;
		}
	}
	
	return NULL;
}

static int SvpBindListenerIpv4(SvpListener *listener, SvpEndpointIpv4 *endpoint)
{
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = endpoint->Address;
	addr.sin_port = htons(endpoint->Port);

	if (bind(listener->Socket, (SOCKADDR *)&addr, sizeof(addr))) {
		return 1;
	}
	
	return 0;
}

static int SvpBindListener(SvpListener *listener, SvpEndpoint *endpoint)
{
	switch (endpoint->Type) {
		case SVP_ENDPOINT_IPV4:
			return SvpBindListenerIpv4(listener, (SvpEndpointIpv4 *)endpoint);
		default:
			assert(0);
	}
}

static SvpListener * SvpCreateListener(SvpEndpoint *endpoint)
{
	SvpListener *listener;
	SOCKET socketHandle;
	
	listener = RtlAllocateHeap(sizeof(SvpListener));
	if (listener == NULL) {
		return NULL;
	}
	
	socketHandle = socket(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP);
	if (socketHandle == INVALID_SOCKET) {
		RtlFreeHeap(listener);
		return NULL;
	}
	
	InsertHeadList(&SvpListenerList, &listener->Entry);
	listener->Endpoint = endpoint;
	listener->Socket = socketHandle;
	InitializeListHead(&listener->SiteBlockList);
	
	if (SvpBindListener(listener, endpoint)) {
		RemoveEntryList(&listener->Entry);
		closesocket(socketHandle);
		RtlFreeHeap(listener);
		return NULL;
	}
	
	return listener;
}

int SvpCreateListeners(void)
{
	SvSite *site;
	SvpEndpoint *endpoint;
	SvpListener *listener;
	SvpSiteBlock *siteBlock;
	
	while (!IsListEmpty(&SvpSiteList)) {
		site = CONTAINING_RECORD(SvpSiteList.Flink, SvSite, Entry);
		RemoveEntryList(&site->Entry);
		
		while (!IsListEmpty(&site->EndpointList)) {
			endpoint = CONTAINING_RECORD(site->EndpointList.Flink, SvpEndpoint, Entry);
			RemoveEntryList(&endpoint->Entry);
			
			listener = SvpFindListener(endpoint);
			if (listener == NULL) {
				listener = SvpCreateListener(endpoint);
				if (listener == NULL) {
					// Deliberately leak resources
					return 1;
				}
				
				// The endpoint was used as part of the listener
			} else {
				RtlFreeHeap(endpoint);
			}
			
			siteBlock = RtlAllocateHeap(sizeof(SvpSiteBlock));
			if (siteBlock == NULL) {
				// Deliberately leak resources
				return 1;
			}
			
			siteBlock->Site = site;
			InsertTailList(&listener->SiteBlockList, &siteBlock->Entry);
		}
	}
	
	return 0;
}

int SvpStartListeners(void)
{
	ListEntry *entry;
	SvpListener *listener;
	
	for (entry = SvpListenerList.Flink; entry != &SvpListenerList; entry = entry->Flink) {
		listener = CONTAINING_RECORD(entry, SvpListener, Entry);
		
		if (WSAEventSelect(listener->Socket, SvpAcceptEvent, FD_ACCEPT)) {
			return 1;
		}
		
		if (listen(listener->Socket, SOMAXCONN)) {
			return 1;
		}
		
		if (SvpAcceptConnection(listener)) {
			return 1;
		}
	}
	
	return 0;
}

int SvpAcceptListeners(void)
{
	ListEntry *entry;
	SvpListener *listener;
	WSANETWORKEVENTS events;
	
	for (entry = SvpListenerList.Flink; entry != &SvpListenerList; entry = entry->Flink) {
		listener = CONTAINING_RECORD(entry, SvpListener, Entry);
		
		if (WSAEnumNetworkEvents(listener->Socket, NULL, &events)) {
			return 1;
		}
		
		if ((events.lNetworkEvents & FD_ACCEPT) && SvpAcceptConnection(listener)) {
			return 1;
		}
	}
}
