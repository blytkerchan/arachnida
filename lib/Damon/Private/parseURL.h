#ifndef _damon_private_parseurl_h
#define _damon_private_parseurl_h

#include "../Details/prologue.h"
#include <string>
#include <boost/cstdint.hpp>
#include <boost/tuple/tuple.hpp>

namespace Damon
{
	namespace Private
	{
		DAMON_API boost::tuple< std::string, std::string, boost::uint16_t, std::string > parseURL(const std::string & url);
		DAMON_API std::string extractHost(const std::string & url);
	}
}

#endif
