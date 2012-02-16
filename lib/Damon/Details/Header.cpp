#include "Header.h"
#include <Acari/base64encode.h>

namespace Damon
{
	namespace Details
	{
		DAMON_API Header createBasicAuthorizationHeader(const std::string & username, const std::string & password)
		{
			return Header("Authorization", "Basic " + Acari::base64encode(username + ":" + password));
		}
	}
}
