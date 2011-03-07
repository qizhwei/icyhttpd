#ifndef _FCGISESSION_H
#define _FCGISESSION_H

#include "Types.h"
#include "FcgiPool.h"

namespace Httpd
{
	class FcgiSession: NonCopyable
	{
	public:
		FcgiSession(FcgiPool &pool);
		~FcgiSession();

	private:

	};
}

#endif