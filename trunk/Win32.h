#ifndef _WIN32_H
#define _WIN32_H

#if _WIN32_WINNT < 0x0501
#define _WIN32_WINNT 0x0501
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <WinSock2.h>
#include <MSWSock.h>
#include <ws2ipdef.h>

#endif
