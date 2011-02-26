#ifndef _HANDLER_H
#define _HANDLER_H

#include "Types.h"

namespace Httpd
{
	class Handler: NonCopyable
	{
	public:
		// TODO: Handler interface
		virtual ~Handler() {}
	};
}

#endif
