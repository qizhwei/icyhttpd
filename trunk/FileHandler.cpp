#include "FileHandler.h"
#include "Http.h"
#include "Node.h"
#include "Types.h"
#include "Exception.h"
#include "Utility.h"
#include <cstring>
#include <string>
#include <vector>

using namespace std;

namespace Httpd
{
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
			}
			Win32Handle file(hFile);

			// TODO: range, multi-range, directory, default file
			UInt64 fileSize = GetFileSize(file.Handle());
			response.BeginHeader("200 OK");
			response.AppendHeader("Content-Length", fileSize);
			response.EndHeader(true);

			if (!head)
				response.TransmitFile(file.Handle(), 0, 0);
		}
		response.Write(nullptr, 0);
	}
}
