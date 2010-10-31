#ifndef _SV_H
#define _SV_H

typedef struct _SvSite SvSite;

SvSite * SvCreateSite(const char *host);
int SvBindEndpointIpv4(SvSite *site, const char *ip, int port);
int SvThreadEntry(void);

#endif
