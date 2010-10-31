#include "sv.h"
#include <stddef.h>

int main(int argc, char **argv)
{
	SvSite *site = SvCreateSite(NULL);
	SvBindEndpointIpv4(site, NULL, 81);
	return SvThreadEntry();
}
