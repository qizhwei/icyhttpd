#include "FcgiHandler.h"
#include "Node.h"
#include "Http.h"
#include "Exception.h"
#include "FcgiProcess.h"
#include "FcgiSession.h"
#include "Stream.h"
#include "Constant.h"
#include <string>
#include <memory>

using namespace Httpd;
using namespace std;

namespace Httpd
{
	void FcgiHandler::Handle(Node &node, HttpRequest &request, HttpResponse &response)
	{
		// TODO: use FcgiPool
		auto_ptr<FcgiProcess> fp(FcgiProcess::Create(L"E:\\Tools\\php-5.3.5-nts-Win32-VC9-x86\\php-cgi.exe", 500));
		SharedPtr<FcgiSession> fs(new FcgiSession(*fp));

		{
			const char *uri = request.URI();
			// TODO: what if uri == "*"?
			string scriptFilename = node.PathA() + uri;
			fs->WriteParam("SCRIPT_FILENAME", scriptFilename.c_str());
		}
		fs->WriteParam("REQUEST_METHOD", request.Method());
		fs->WriteParam("SERVER_NAME", "icyhttpd");
		fs->CloseParam();
		fs->CloseStdin();

		Reader<FcgiSession> fsReader(*fs);
		BufferedReader reader(fsReader, BufferBlockSize);

		char *header;
		if ((header = reader.ReadLine()) == nullptr)
			throw SystemException();

		if (!_strnicmp(header, "Status:", 7)) {
			header += 7;
			while (*header == ' ' || *header == '\t')
				++header;
			response.BeginHeader(header);
		} else {
			response.BeginHeader("200 OK");
			response.AppendHeader(header);
		}

		reader.Flush();
		while (true) {
			if ((header = reader.ReadLine()) == nullptr)
				throw SystemException();

			if (*header == '\0')
				break;

			response.AppendHeader(header);
			reader.Flush();
		}

		// TODO: Is length really not provided?
		response.EndHeader(false);

		char buffer[BufferBlockSize];
		UInt32 size;
		
		while ((size = reader.Read(buffer, sizeof(buffer))) != 0) {
			response.Write(buffer, size);
		}

		response.Write(nullptr, 0);
	}
}
