#include "Exception.h"
#include "Http.h"
#include "Types.h"

namespace Httpd
{
	void HttpException::BuildResponse(HttpResponse &response) throw()
	{
		const char *status;
		size_t statusLength;
		size_t additionLength;

		try {
			switch (this->statusCode)
			{
			case 400:
				status = "400 Bad Request";
				break;
			case 403:
				status = "403 Forbidden";
				break;
			case 404:
				status = "404 Not Found";
				break;
			case 413:
				status = "413 Request Entity Too Large";
				break;
			case 501:
				status = "501 Not Implemented";
				break;
			case 505:
				status = "505 Http Version Not Supported";
				break;
			default:
				this->mustClose = true;
				return;
			}

			response.BeginHeader(status);
			statusLength = strlen(status);
			if (this->addition != nullptr) {
				additionLength = strlen(this->addition);
			} else {
				additionLength = 0;
			}
			response.AppendHeader("Content-Length", 4 + statusLength + additionLength + 7);
			response.EndHeader(true);
			response.Write("<h1>", 4);
			response.Write(status, statusLength);
			if (additionLength != 0)
				response.Write(this->addition, additionLength);
			response.Write("</h1>\r\n", 7);
			response.Write(nullptr, 0);
		} catch (const std::exception &) {
			this->mustClose = true;
		}
	}
}
