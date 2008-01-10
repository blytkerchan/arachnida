#include "Request.h"

namespace Damon
{
	Request::Request(const std::string & url, Method method/* = get__*/)
	{ /* no-op */ }
	Request::~Request()
	{ /* no-op */ }

	/*DAMON_API */Response send(const Request & request)
	{
		return Response();
	}
}
