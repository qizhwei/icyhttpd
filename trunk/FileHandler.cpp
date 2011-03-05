#include "FileHandler.h"
#include "Types.h"
#include "Exception.h"
#include "Utility.h"
#include <vector>

using namespace std;

namespace Httpd
{
	void FileHandler::Handle(Node &node, HttpRequest &request, HttpResponse &response)
	{
		const char *path = node.Path();
		int pathSize = MultiByteToWideChar(CP_ACP, 0, path, -1, NULL, 0);
		const char *uri = request.URI();
		int uriSize = MultiByteToWideChar(CP_UTF8, 0, uri, -1, NULL, 0);

		if (pathSize == 0 || uriSize == 0)
			throw SystemException();

		vector<wchar_t> name(pathSize + uriSize - 1);
		MultiByteToWideChar(CP_ACP, 0, path, -1, &name[0], pathSize);
		MultiByteToWideChar(CP_UTF8, 0, uri, -1, &name[pathSize - 1], uriSize);

		Win32Handle file(OpenFile(&name[0]));

		// TODO: Range, directory, block size <= UINT32_MAX
		UInt64 fileSize = GetFileSize(file.Handle());
		response.AppendHeader("Content-Length", fileSize);
		response.EndHeader(200, "OK", true);
		response.TransmitFile(file.Handle(), 0, fileSize);
	}
}
