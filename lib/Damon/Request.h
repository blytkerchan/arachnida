#ifndef _damon_request_h
#define _damon_request_h

#include "Details/prologue.h"
#include <string>
#include "Response.h"

namespace Damon
{
	class DAMON_API Request
	{
	public :
		enum Method { options__, get__, head__, post__, put__, delete__, trace__, connect__ };

		Request(const std::string & url, Method method = get__);
		~Request();
	};

	DAMON_API Response send(const Request & request);
}

#endif
