#ifndef _damon_response_h
#define _damon_response_h

#include <string>
#include <vector>
#include "Details/Header.h"

namespace Damon
{
	struct Response
	{
		int status_code_;							///< the status code for the response
		std::string reason_string_;					///< the human-readable equivalent of the status code
		std::string protocol_and_version_;			///< the protocol and version of the original request (see Sections 6, 3.1 and 10.5.6)
		Details::HeaderFields header_fields_;		///< all header fields found while parsing the request
		std::vector< char > body_;					///< the body of the request
	};
}

#endif
