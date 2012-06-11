#ifndef _HANDLER_H
#define _HANDLER_H

#include "daemon.h"

extern DM_HANDLER_TYPE StaticHandlerType;
extern DM_HANDLER_TYPE FcHandlerType;

static DM_HANDLER_TYPE *DmHandlerList[] = {
	&StaticHandlerType,
	&FcHandlerType,
	NULL,
};

#endif
