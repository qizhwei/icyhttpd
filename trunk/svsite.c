#include "svp.h"
#include "rtl.h"

SvSite * SvCreateSite(const char *host)
{
	SvSite *site;
	
	if (SvpInitialize()) {
		return NULL;
	}
	
	site = RtlAllocateHeap(sizeof(SvSite), "SvCreateSite");
	if (site == NULL) {
		return NULL;
	}
	
	if (host == NULL) {
		site->Host = NULL;
	} else {
		site->Host = RtlDuplicateString(host);
		if (site->Host == NULL) {
			RtlFreeHeap(site);
			return NULL;
		}
	}
	
	InsertTailList(&SvpSiteList, &site->Entry);
	InitializeListHead(&site->EndpointList);
	
	return site;
}

int SvBindEndpointIpv4(SvSite *site, const char *ip, int port)
{
	SvpEndpointIpv4 *endpoint;
	unsigned long address = INADDR_ANY;
	
	if (ip != NULL && ip[0] != '\0') {
		address = inet_addr(ip);
		if (address == INADDR_NONE) {
			return 1;
		}
	}
	
	endpoint = RtlAllocateHeap(sizeof(SvpEndpointIpv4), "SvBindEndpointIpv4");
	if (endpoint == NULL) {
		return 1;
	}
	
	endpoint->Header.Type = SVP_ENDPOINT_IPV4;
	InsertTailList(&site->EndpointList, &endpoint->Header.Entry);
	endpoint->Address = address;
	endpoint->Port = port;
	return 0;
}
