#ifndef _FASTCGI_H
#define _FASTCGI_H

#include "Types.h"
#include "Utility.h"
#include "Handler.h"
#include "Dispatcher.h"
#include "fcpool.h"

namespace Httpd
{
	class FcObject
	{
	public:
		FcObject();
		FcObject(const FcObject &o);
		FcObject(void *u);
		FcObject &operator =(FcObject &o);
		~FcObject();
		void Pointer(void *u);
		void *Pointer();
	private:
		void *u;
	};

	class FcPool: public FcObject
	{
	public:
		FcPool(const char *cmdLine, int queueLength, int maxInstances,
			int idleTime, int maxRequests);
	};

	class FcRequest: public FcObject
	{
	public:
		FcRequest();
		void Begin(FcPool *pool);
		void Abort();
		UInt32 Read(char *buffer, UInt32 size);
		UInt32 Write(const char *buffer, UInt32 size);
		void WriteParam(const char *name, const char *value);
		void WriteParam();
	private:
		UInt32 WriteParamInternal(const char *buffer, UInt32 size);
	};

	class FcgiHandler: public Handler
	{
	public:
		FcgiHandler(const char *cmdLine, int queueLength, int maxInstances,
			int idleTime, int maxRequests);
		virtual void Handle(Node &node, HttpRequest &request, HttpResponse &response);
	private:
		~FcgiHandler();
	private:
		FcPool pool;
	};
}

#endif
