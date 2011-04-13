#ifndef _FASTCGI_H
#define _FASTCGI_H

#include "Types.h"
#include "Utility.h"
#include <list>
#include <string>

namespace Httpd
{
	class FcgiPool: public NonCopyable
	{
		friend class FcgiProcess;
	public:
		FcgiPool(std::string commandLine);

	private:
		std::list<FcgiProcess *> idleList;
		std::list<FcgiProcess *> busyList;
		ThreadLock lock;

		// this class has no destructor, declaration only
		~FcgiPool();
	};

	class FcgiProcess: public NonCopyable
	{
	};

	class FcgiSession: public NonCopyable
	{
	};
}

#endif
