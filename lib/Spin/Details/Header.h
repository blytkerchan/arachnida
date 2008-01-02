#ifndef _spin_details_header_h
#define _spin_details_header_h

#include <list>

namespace Spin
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
	}
}

#endif
