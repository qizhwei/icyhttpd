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
		FcgiProcess *Pop();
		void Push(FcgiProcess *process);

	private:
		ThreadLock lock;
		std::list<FcgiProcess *> idleList;

	private:
		std::string commandLine;

	private:
		// this class has no destructor, declaration only
		~FcgiPool();
	};

	class FcgiProcess: public NonCopyable
	{
	private:
		FcgiProcess(FcgiPool &pool);

	private:
		FcgiPool &pool;
	};

	class FcgiSession: public NonCopyable
	{
	};
}

#endif
