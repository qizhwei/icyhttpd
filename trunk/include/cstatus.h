#ifndef HTTPD_CSTATUS_H
#define HTTPD_CSTATUS_H

typedef int CSTATUS;

#define SUCCESS(x) ((x) >= 0)

#define C_SUCCESS (0)
#define C_GENERIC_ERROR (-1)
#define C_NOT_IMPLEMENTED (-2)
#define C_OS_ERROR (-3)
#define C_SOCKET_ERROR (-4)
#define C_LIBC_ERROR (-5)
#define C_BAD_ALLOC (-6)
#define C_INVALID_PORT (-7)
#define C_INVALID_IP_ADDRESS (-8)
#define C_LIMIT_EXCEEDED (-9)
#define C_END_OF_FILE (-10)
#define C_BAD_REQUEST (-11)
#define C_HTTP_VERSION_NOT_SUPPORTED (-12)
#define C_INVALID_HOSTNAME (-13)
#define C_UNSUPPORTED_FILENAME (-14)
#define C_REQUEST_ENTITY_TOO_LARGE (-15)
#define C_ALREADY_EXISTS (-16)
#define C_INVALID_NAME (-17)
#define C_ACCESS_DENIED (-18)
#define C_NOT_FOUND (-19)
#define C_CONNECTION_BROKEN (-20)

#endif
