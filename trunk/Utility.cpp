#include "Utility.h"
#include "Types.h"
#include "Exception.h"
#include "Constant.h"
#include <utility>
#include <string>
#include <vector>

using namespace Httpd;
using namespace std;

namespace Httpd
{
	HANDLE OpenFile(const wchar_t *path)
	{
		HANDLE hFile;

		if ((hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN, NULL)) == INVALID_HANDLE_VALUE)
		{
			DWORD lastErr = GetLastError();
			switch (lastErr) {
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
			case ERROR_INVALID_NAME:
				throw NotFoundException();
			case ERROR_ACCESS_DENIED:
				throw ForbiddenException();
			default:
				throw SystemException();
			}
		}

		return hFile;
	}

	UInt64 GetFileSize(HANDLE hFile)
	{
		LARGE_INTEGER liFileSize;
		if (!GetFileSizeEx(hFile, &liFileSize))
			throw SystemException();
		return liFileSize.QuadPart;
	}

	wstring MB2WC(UINT codePage, const string &s)
	{
		size_t originalSize;
		size_t convertedSize;

		if ((originalSize = s.size()) == 0)
			return wstring();

		if ((convertedSize = MultiByteToWideChar(codePage, 0, s.c_str(), originalSize, NULL, 0)) == 0)
			throw SystemException();

		vector<wchar_t> buffer(convertedSize);
		MultiByteToWideChar(codePage, 0, s.c_str(), originalSize, &*buffer.begin(), convertedSize);
		return wstring(buffer.begin(), buffer.end());
	}

	char *ParseCommaList(char *&next)
	{
		char *first = next, *last;
	
		if ((next = strchr(first, ',')) != nullptr) {
			last = next;
			*next++ = '\0';
		} else {
			last = first + strlen(first);
		}

		EatLWS(first);
		EatLWSBackwards(first, last);

		return first;
	}

	
	UInt64 ParseUInt64Dec(char *p)
	{
		while (*p == '0')
			++p;

		UInt64 u = 0;
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
}
