#include "FileHandler.h"
#include "Http.h"
#include "Node.h"
#include "Types.h"
#include "Exception.h"
#include "Utility.h"
#include <cstring>
#include <string>
#include <vector>

using namespace Httpd;
using namespace std;

namespace
{
	vector<pair<UInt64, UInt64> > ParseRanges(char *range, UInt64 totalBytes)
	{
		vector<pair<UInt64, UInt64> > result;

		if (range == nullptr)
			return result;
		if (_strnicmp(range, "bytes", 5) != 0)
			return result;
		range += 5;
		if (*range++ != '=')
			return result;

		do {
			char *cur = ParseCommaList(range);
			char *bar = strchr(cur, '-');
			if (bar == nullptr) {
				result.clear();
				return result;
			}

			if (bar != cur) {
				*bar++ = '\0';
				UInt64 first, second;
				first = ParseUInt64Dec(cur);
				if (*bar != '\0') {
					second = ParseUInt64Dec(bar);
				} else {
					second = totalBytes - 1;
				}
				if (first == UINT64_MAX || second == UINT64_MAX) {
					result.clear();
					return result;
				}
				if (second < first || second >= totalBytes)
					throw HttpException(416, false, nullptr);
				result.push_back(make_pair(first, second));
			} else {
				++bar;
				UInt64 length = ParseUInt64Dec(bar);
				if (length == UINT64_MAX) {
					result.clear();
					return result;
				}
				if (length == 0 || length > totalBytes)
					throw HttpException(416, false, nullptr);
				result.push_back(make_pair(totalBytes - length, totalBytes - 1));
			}
		} while (range != nullptr);

		return result;
	}
}

namespace Httpd
{
	FileHandler &FileHandler::Instance()
	{
		static FileHandler *fh = new FileHandler;
		return *fh;
	}

	FileHandler::FileHandler()
	{
	}

	void FileHandler::Handle(Node &node, HttpRequest &request, HttpResponse &response)
	{
		bool head;

		if (!strcmp(request.Method(), "GET"))
			head = false;
		else if (!strcmp(request.Method(), "HEAD"))
			head = true;
		else
			throw HttpException(501, false, nullptr);

		const wstring path = node.PathW();
		size_t pathSize = path.size();
		const char *uri = request.URI();
		if (*uri != '/')
			throw HttpException(400, false, nullptr);
		int uriSize;
		if ((uriSize = MultiByteToWideChar(CP_UTF8, 0, uri, -1, NULL, 0)) == 0)
			throw SystemException();

		// N.B. uriSize includes null terminate character
		vector<wchar_t> name(pathSize + uriSize);
		memcpy(&name[0], path.c_str(), pathSize * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, uri, -1, &name[pathSize], uriSize);

		{
			HANDLE hFile;

			try {
				hFile = OpenFile(&name[0]);
			} catch (const ForbiddenException &) {
				throw HttpException(403, false, nullptr);
			} catch (const NotFoundException &) {
				throw HttpException(404, false, nullptr);
			} catch (const SystemException &) {
				throw HttpException(500, false, nullptr);
			}

			// TODO: directory, default file
			Win32Handle file(hFile);
			UInt64 fileSize = GetFileSize(file.Handle());
			auto ranges = ParseRanges(request.Header("Range"), fileSize);

			// TODO: Multiple ranges not implemented
			if (ranges.size() != 1) {
				response.BeginHeader("200 OK");
				response.AppendHeader("Content-Length", fileSize);
				response.EndHeader(true);

				if (!head)
					response.TransmitFile(file.Handle());
			} else {
				response.BeginHeader("206 Partial Content");
				auto range = ranges.begin();
				response.AppendHeader("Content-Range", "bytes ", range->first, range->second, fileSize);
				response.AppendHeader("Content-Length", range->second - range->first + 1);
				response.EndHeader(true);

				if (!head)
					response.TransmitFile(file.Handle(), *range);
			}
		}
		response.Write(nullptr, 0);
	}
}
