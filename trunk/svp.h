#ifndef _SVP_H
#define _SVP_H

#include "sv.h"
#include "rtl.h"
#include "win32.h"

extern ListEntry SvpSiteList;
extern ListEntry SvpListenerList;
extern HANDLE SvpAcceptEvent;

struct _SvSite {
	char *Host;
	ListEntry Entry;
	ListEntry EndpointList;
};

typedef struct _SvpEndpoint {
	int Type;
	ListEntry Entry;
} SvpEndpoint;

#define SVP_ENDPOINT_IPV4 (0)

typedef struct _SvpEndpointIpv4 {
	SvpEndpoint Header;
	unsigned long Address;
	int Port;
} SvpEndpointIpv4;

typedef struct _SvpListener {
	ListEntry Entry;
	SvpEndpoint *Endpoint;
	SOCKET Socket;
	ListEntry SiteBlockList;
} SvpListener;

typedef struct _SvpSiteBlock {
	SvSite *Site;
	ListEntry Entry;
} SvpSiteBlock;

extern int SvpCreateListeners(void);
extern int SvpStartListeners(void);
extern int SvpAcceptListeners(void);
extern int SvpAcceptSession(SvpListener *listener);

#endif
