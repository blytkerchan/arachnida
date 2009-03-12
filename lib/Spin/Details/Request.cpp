#include "Request.h"

namespace Spin
{
	namespace Details
	{
		std::string Request::getHeaderValue(const std::string & header_name) const
		{
			std::string retval;
			bool found_one(false);

			for (HeaderFields::const_iterator curr(header_fields_.begin()); curr != header_fields_.end(); ++curr)
			{
				if (curr->name_ == header_name)
				{
					retval += (found_one ? "," : "") + curr->value_;
				}
				else
				{ /* no-op */ }
			}

			return retval;
		}
	}
}

