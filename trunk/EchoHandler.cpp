#include "EchoHandler.h"
#include "Constant.h"
#include "Stream.h"

namespace Httpd
{
	void EchoHandler::Handle(Node &node, HttpRequest &request, HttpResponse &response)
	{
		response.AppendHeader("Content-Type: text/plain; charset=utf-8\r\n");
		response.EndHeader(200, "OK", false);

		Writer<HttpResponse> responseWriter(response);
		BufferedWriter writer(responseWriter, BufferBlockSize);
		writer.AppendLine("icyhttpd echo handler");
		writer.AppendLine();
		writer.AppendLine("[Information]");
		writer.Append("Host: ");
		writer.AppendLine(request.Host());
		writer.Append("URI: ");
		writer.AppendLine(request.URI());
		writer.Append("Query string: ");
		writer.AppendLine(request.QueryString());
		writer.Append("Method: ");
		writer.AppendLine(request.Method());
		writer.Append("Client version: ");
		writer.Append(request.Version().first);
		writer.Append(".");
		writer.AppendLine(request.Version().second);
		writer.Append("Entity length: ");
		if (request.Chunked())
			writer.AppendLine("chunked");
		else
			writer.AppendLine(request.RemainingLength());
		writer.Append("Keep alive: ");
		writer.AppendLine(request.KeepAlive());
		writer.AppendLine();
		writer.AppendLine("[Other Headers]");
		for (UInt32 i = 0; i != request.HeaderCount(); ++i) {
			HttpHeader header = request.GetHeader(i);
			writer.Append(header.first);
			writer.Append(": ");
			writer.AppendLine(header.second);
		}

		writer.Flush();
		response.Write(nullptr, 0);
	}
}
