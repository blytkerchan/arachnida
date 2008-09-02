#ifndef _damon_details_header_h
#define _damon_details_header_h

#include <list>
#include "prologue.h"
#include <string>
#include <vector>

namespace Damon
{
	namespace Details
	{
		//! An HTTP header: a name-value pair
		struct Header
		{
			Header()
			{ /* no-op */ }

			Header(const std::string & name, const std::string & value)
				: name_(name),
				  value_(value)
			{ /* no-op */ }

			std::string name_;	///< the name
			std::string value_;	///< the value
		};
		typedef std::list< Header > HeaderFields;	///< the type of the list of header fields
		
		//! Create a header for basic HTTP authorization.
		DAMON_API Header createBasicAuthorizationHeader(const std::string & username, const std::string & password);
	}
}

#endif
