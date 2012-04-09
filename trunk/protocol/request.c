#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "protocol.h"

#define READER_READ_SIZE (4096)
#define READER_MAX_ALLOC_SIZE (32768)

static uint16_t ParseUInt16(char **p)
{
	int i = 0;
	while (**p >= '0' && **p <= '9') {
		if ((i = i * 10 + (**p - '0')) > UINT16_MAX)
			i = UINT16_MAX;
		++*p;
	}
	return i;
}

uint64_t ParseUInt64Dec(char *p)
{
	uint64_t u = 0;

	while (*p == '0')
		++p;

	while (*p != '\0') {
		if (*p >= '0' && *p <= '9') {
			if (u > 1844674407370955161U)
				return UINT64_MAX;
			else if (u == 1844674407370955161U && *p > '5')
				return UINT64_MAX;
			u = u * 10 + (*p - '0');
		} else {
			return UINT64_MAX;
		}
		++p;
	}

	return u;
}


static uint64_t ParseUInt64Hex(char *p)
{
	uint16_t u = 0;

	while (*p == '0')
		++p;

	while (*p != '\0') {
		if (u > 0xfffffffffffffffU)
			return UINT64_MAX;
		else if (*p >= '0' && *p <= '9')
			u = (u << 4) | (*p - '0');
		else if (*p >= 'a' && *p <= 'f')
			u = (u << 4) | (*p - 'a' + 10);
		else if (*p >= 'A' && *p <= 'F')
			u = (u << 4) | (*p - 'A' + 10);
		else
			return UINT64_MAX;
		++p;
	}

	return u;
}

static void EatLWS(char **p)
{
	while (**p == ' ' || **p == '\t')
		++*p;
}

static void EatLWSBackwards(char *first, char **last)
{
	while (first != *last && ((*last)[-1] == ' ' || (*last)[-1] == '\t'))
		*--*last = '\0';
}

static char *ParseCommaList(char **next)
{
	char *first = *next, *last;

	if ((*next = strchr(first, ',')) != NULL) {
		last = *next;
		*(*next)++ = '\0';
	} else {
		last = first + strlen(first);
	}

	EatLWS(&first);
	EatLWSBackwards(first, &last);

	return first;
}


static void URIDecode(char *first, char **last)
{
	char *cur = first;
	char state = 0;
	char escape;

	while (*first != '\0') {
		*cur = *first;
		if (state == 0) {
			state = (*cur == '%');
		} else {
			if (*cur >= '0' && *cur <= '9')
				escape = (escape << 4) | (*cur - '0');
			else if (*cur >= 'A' && *cur <= 'F')
				escape = (escape << 4) | (*cur - 'A' + 10);
			else if (*cur >= 'a' && *cur <= 'f')
				escape = (escape << 4) | (*cur - 'a' + 10);
			else
				state = 0;
			if (state == 2) {
				cur -= 2;
				*cur = escape;
			}
			state = (state & 1) << 1;
		}

		++cur;
		++first;
	}

	*(*last = cur) = '\0';
}

static void URIRewrite(char *first, char **last)
{
	char *cur = first;
	int state = 0;

	while (1) {
		if (*first == '\\')
			*first = '/';

		if (*first == '/' || *first == '\0') {
			if (state == 1) {
				cur -= 2;
			} else if (state == 2) {
				cur -= 3;
				while (*cur != '\0')
					if (*--cur == '/')
						break;
				if (*cur == '\0')
					++cur;
			}
			state = 0;
		} else if (*first == '.') {
			++state;
		} else {
			state = INT_MIN;
		}

		if ((*cur = *first) == '\0')
			break;
		++cur;
		++first;
	}

	*last = cur;
}

static int URIValidateWin32(char *first)
{
	// This is a table to filter out con, nul, aux, prn, com[0-9] and lpt[0-9]
	// And this is an excellent place to explain where to write comment

	static char table[][5] = {
		{'n', 'a', 'p', 'l', 'c'}, // 0
		{'\0', '\0', '\0', '\0', 'u'}, // 1
		{'\0', '\0', '\0', '\0', 'u'}, // 2
		{'\0', '\0', '\0', '\0', 'r'}, // 3
		{'\0', '\0', '\0', '\0', 'p'}, // 4
		{'\0', '\0', '\0', '\0', 'o'}, // 5
		{'\0', '\0', '\0', '\0', 'l'}, // 6
		{'\0', '\0', '\0', 'x', '\0'}, // 7
		{'\0', '\0', 'n', '\0', '\0'}, // 8
		{'\0', '\0', 't', '\0', '\0'}, // 9
		{'n', 'm', '\0', '\0', '\0'}, // 10
	};

	int state = 0;
	int i;

	while (1) {
		char ch = *first;
		if (ch == '\0' || ch == '/') {
			if (state == 11) {
				return 0;
			} else if (ch == '\0') {
				return 1;
			}
			state = 0;
			goto bed;
		} else if (ch == '.' || ch == ' ') {
			state = 11;
			goto bed;
		} else if (ch >= 'A' && ch <= 'Z') {
			ch += ('a' - 'A');
		}
		if (state >= 0 && state <= 10) {
			for (i = 0; i < 5; ++i) {
				if (ch == table[state][i]) {
					state += (i + 1);
					goto bed;
				}
			}
			state = 13;
		} else if (state == 12 && (ch >= '0' && ch <= '9')) {
			state = 11;
		} else {
			state = 13;
		}
bed:
		++first;
	}
}

static int HeaderNameCompare(void *Context, const void *Header1, const void *Header2)
{
	const char *base = (const char *)Context;
	HTTP_HEADER *header1 = (HTTP_HEADER *)Header1;
	HTTP_HEADER *header2 = (HTTP_HEADER *)Header2;
	return _stricmp(base + header1->Name, base + header2->Name);
}

static int HeaderNameCompare2(void *Context, const void *HeaderName, const void *Header)
{
	const char *base = (const char *)Context;
	const char *headerName = (const char *)HeaderName;
	HTTP_HEADER *header = (HTTP_HEADER *)Header;
	return _stricmp(headerName, base + header->Name);
}

CSTATUS HttpInitRequest(
	OUT HTTP_REQUEST *Request,
	BUFFERED_READER *Reader)
{
	int done = 0;
	int title = 1;
	int valid = 1;
	CSTATUS status;
	size_t offset;
	char *base;
	char *first;
	char *second;
	char *uri;
	char *uriLast;
	char *host;
	char *colon;
	char *query;
	char *ext;
	char c;
	char *hdrHost;
	char *hdrContentLength;
	char *hdrTransferEncoding;
	char *hdrConnection;
	char *option;

	Request->Reader = Reader;
	Request->Chunked = 0;
	Request->RemainingLength = 0;
	Request->HeaderCount = 0;

	while (!done) {
		// Read request line
		status = BufferedReaderReadLine(Reader, READER_READ_SIZE, READER_MAX_ALLOC_SIZE, 1, &offset);
		if (!SUCCESS(status))
			return status;

		base = Reader->Buffer.Data;
		first = &base[offset];

		if (title) {
			// Empty lines are ignored
			if (first[0] == '\0')
				continue;

			// Request method
			Request->Method = first - base;
			if ((first = strchr(first, ' ')) == NULL)
				return C_BAD_REQUEST;
			*first++ = '\0';

			// Version
			uri = first;
			if ((first = strchr(first, ' ')) == NULL) {
				uriLast = uri + strlen(uri);
				Request->MajorVersion = 0;
				Request->MinorVersion = 9;
				Request->KeepAlive = 0;

				// HTTP/0.9 requests don't have headers
				done = 1;
			} else {
				uriLast = first;
				*first++ = '\0';
				if (strncmp(first, "HTTP/", 5))
					return C_BAD_REQUEST;
				first += 5;
				Request->MajorVersion = ParseUInt16(&first);
				if (*first++ != '.')
					return C_BAD_REQUEST;
				Request->MinorVersion = ParseUInt16(&first);
				if (*first != '\0')
					return C_BAD_REQUEST;

				if (Request->MajorVersion != 1)
					return C_HTTP_VERSION_NOT_SUPPORTED;
				if (Request->MinorVersion == 0)
					Request->KeepAlive = 0;
				else
					Request->KeepAlive = 1;
			}

			// Request URI
			if (uri[0] == '/' || (uri[0] == '*' && uri[1] == '\0')) {
				Request->Host = SIZE_MAX;
			} else {
				if (_strnicmp(uri, "http://", 7))
					return C_BAD_REQUEST;
				host = uri;
				if ((uri = strchr(uri + 7, '/')) == NULL)
					uri = uriLast;
				host[uri - host - 7] = '\0';
				if (uri == uriLast)
					*--uri = '/';
				uri[-1] = '\0';

				if ((colon = strchr(host, ':')) != NULL)
					*colon = '\0';
				Request->Host = host - base;
			}

			// Query string
			if ((query = strchr(uri, '?')) != NULL) {
				uriLast = query;
				*query++ = '\0';
				Request->QueryString = query - base;
			} else {
				Request->QueryString = SIZE_MAX;
			}

			URIDecode(uri, &uriLast);
			URIRewrite(uri, &uriLast);
			valid = URIValidateWin32(uri);

			// Extension
			assert(uri[-1] == '\0');
			ext = uriLast;
			while (1) {
				c = *--ext;
				if (c == '.') {
					break;
				} else if (c == '/' || c == '\0') {
					ext = NULL;
					break;
				}
			}

			Request->URI = uri - base;
			Request->Extension = (ext != NULL) ? (ext - base) : SIZE_MAX;
			title = 0;
		} else {
			if (*first == '\0') {
				done = 1;
			} else if (*first == ' ' || *first == '\t') {
				// Ignore the standard
				return C_BAD_REQUEST;
			} else {
				if ((second = strchr(first, ':')) == NULL)
					return C_BAD_REQUEST;
				*second++ = '\0';

				// Eat leading LWS (traling LWSes already eaten)
				while (*second == ' ' || *second == '\t')
					++second;

				// Record header
				Request->UnsortedHeaders[Request->HeaderCount].Name = first - base;
				Request->UnsortedHeaders[Request->HeaderCount].Value = second - base;
				Request->SortedHeaders[Request->HeaderCount].Name = first - base;
				Request->SortedHeaders[Request->HeaderCount].Value = second - base;
				++Request->HeaderCount;
			}
		}
	}

	// Sort headers for binary searching
	qsort_s(Request->SortedHeaders, Request->HeaderCount, sizeof(*Request->SortedHeaders), HeaderNameCompare, base);

	// Interpret some headers
	if (Request->Host == SIZE_MAX) {
		if ((hdrHost = HttpHeaderByNameRequest(Request, "Host")) != NULL) {
			if ((colon = strchr(hdrHost, ':')) != NULL)
				*colon = '\0';
			Request->Host = hdrHost - base;
		}
	}

	if (Request->MajorVersion == 1 && Request->MinorVersion >= 1 && Request->Host == SIZE_MAX)
		return C_INVALID_HOSTNAME;
	if (!valid)
		return C_UNSUPPORTED_FILENAME;

	if ((hdrContentLength = HttpHeaderByNameRequest(Request, "Content-Length")) != NULL) {
		Request->RemainingLength = ParseUInt64Dec(hdrContentLength);
		if (Request->RemainingLength == UINT64_MAX)
			return C_REQUEST_ENTITY_TOO_LARGE;
	}

	if ((hdrTransferEncoding = HttpHeaderByNameRequest(Request, "Transfer-Encoding")) != NULL
		&& _stricmp(hdrTransferEncoding, "identity")) {
		Request->RemainingLength = 0;
		Request->Chunked = 1;
	}

	if ((hdrConnection = HttpHeaderByNameRequest(Request, "Connection")) != NULL) {
		do {
			option = ParseCommaList(&hdrConnection);
			if (!_stricmp(option, "keep-alive"))
				Request->KeepAlive = 1;
			else if (!_stricmp(option, "close"))
				Request->KeepAlive = 0;
		} while (hdrConnection != NULL);

	}

	return C_SUCCESS;
}

void HttpUninitRequest(
	HTTP_REQUEST *Request)
{
	char buffer[BUFFER_BLOCK_SIZE];
	size_t actualSize;

	// Eat the remaining request body
	while (SUCCESS(HttpReadRequest(Request, buffer, sizeof(buffer), &actualSize)));
	BufferedReaderFlush(Request->Reader);
}

void HttpHeaderByIndexRequest(
	HTTP_REQUEST *Request,
	size_t Index,
	OUT char **HeaderName,
	OUT char **HeaderValue)
{
	char *base = Request->Reader->Buffer.Data;
	HTTP_HEADER *header = &Request->UnsortedHeaders[Index];
	*HeaderName = &base[header->Name];
	*HeaderValue = &base[header->Value];
}

char *HttpHeaderByNameRequest(
	HTTP_REQUEST *Request,
	const char *HeaderName)
{
	char *base = Request->Reader->Buffer.Data;
	HTTP_HEADER *header = (HTTP_HEADER *)bsearch_s(HeaderName,
		Request->SortedHeaders, Request->HeaderCount,
		sizeof(*Request->SortedHeaders), HeaderNameCompare2, base);
	if (header)
		return &base[header->Value];
	else
		return NULL;
}

char *HttpGetMethodRequest(
	HTTP_REQUEST *Request)
{
	return &Request->Reader->Buffer.Data[Request->Method];
}

char *HttpGetURIRequest(
	HTTP_REQUEST *Request)
{
	return &Request->Reader->Buffer.Data[Request->URI];
}

char *HttpGetExtensionRequest(
	HTTP_REQUEST *Request)
{
	size_t offset = Request->Extension;
	if (offset == SIZE_MAX)
		return NULL;
	return &Request->Reader->Buffer.Data[offset];
}

char *HttpGetQueryStringRequest(
	HTTP_REQUEST *Request)
{
	size_t offset = Request->QueryString;
	if (offset == SIZE_MAX)
		return NULL;
	return &Request->Reader->Buffer.Data[offset];
}

char *HttpGetHostRequest(
	HTTP_REQUEST *Request)
{
	size_t offset = Request->Host;
	if (offset == SIZE_MAX)
		return NULL;
	return &Request->Reader->Buffer.Data[offset];
}

CSTATUS HttpReadRequest(
	HTTP_REQUEST *Request,
	char *Buffer,
	size_t Size,
	OUT size_t *ActualSize)
{
	CSTATUS status;
	size_t offset;
	char *line;

	if (Request->RemainingLength == 0) {
		if (Request->Chunked) {
			do {
				BufferedReaderFlush(Request->Reader);
				status = BufferedReaderReadLine(Request->Reader, READER_READ_SIZE, READER_MAX_ALLOC_SIZE, 0, &offset);
				if (!SUCCESS(status))
					return status;
				line = &Request->Reader->Buffer.Data[offset];
			} while (line[0] == '\0');

			if ((Request->RemainingLength = ParseUInt64Hex(line)) == UINT64_MAX)
				return C_BAD_REQUEST;

			if (Request->RemainingLength == 0) {
				Request->Chunked = 0;
				return C_END_OF_FILE;
			}
		} else {
			return C_END_OF_FILE;
		}
	}

	assert(Request->RemainingLength > 0);

	if (Request->RemainingLength < (uint64_t)Size)
		Size = (size_t)Request->RemainingLength;
	status = BufferedReaderRead(Request->Reader, Buffer, Size, ActualSize);
	if (!SUCCESS(status))
		return status;

	BufferedReaderFlush(Request->Reader);
	Request->RemainingLength -= *ActualSize;
	return C_SUCCESS;
}
