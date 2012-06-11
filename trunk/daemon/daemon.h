#ifndef _DAEMON_H
#define _DAEMON_H

#include <stdio.h>
#include <httpd_daemon.h>
#include <httpd_util.h>

extern void DmConfigParserEntry(void);
extern CSTATUS DmInitServer(void);

#endif
