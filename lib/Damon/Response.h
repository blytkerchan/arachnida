#ifndef _damon_response_h
#define _damon_response_h

#include <string>
#include <vector>
#include "Details/Header.h"

namespace Damon
{
	//! A response, as received from the server.
	struct Response
	{
		/** Construct a response.
		 * \param protocol_and_version the protocol and version, normally those of the original request.
		 * \param status_code the status or response code of the query
		 * \param reason_string the reason why the status code is what it is */
		Response(const std::string & protocol_and_version, unsigned int status_code, const std::string & reason_string)
			: protocol_and_version_(protocol_and_version),
			  status_code_(status_code),
			  reason_string_(reason_string)
		{ /* no-op */ }

		std::string protocol_and_version_;			///< the protocol and version of the original request (see Sections 6, 3.1 and 10.5.6)
		unsigned int status_code_;					///< the status code for the response
		std::string reason_string_;					///< the human-readable equivalent of the status code
		Details::HeaderFields header_fields_;		///< all header fields found while parsing the request
		std::vector< char > body_;					///< the body of the request
	};
}

#endif
