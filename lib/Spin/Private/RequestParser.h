#ifndef _spin_private_requestparser_h
#define _spin_private_requestparser_h

#include <string>
#include "../Details/Request.h"

namespace Spin
{
	namespace Private
	{
		Details::Request parse(const std::string & request);
	}
}

#endif
