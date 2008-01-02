#ifndef _spin_details_request_h
#define _spin_details_request_h

#include <string>
#include <vector>
#include <utility>

#include "Header.h"

namespace Spin
{
	class Connection;
	namespace Details
	{
		//! An HTTP request.
		struct Request
		{
			/** Construct a new request with a connection.
			 * \param connection the connection to use to respond to the request
			 * \param method the method of the request, as supplied by the client
			 * \param url the location of the resource requested
			 * \param protocol_and_version the protocol and version used by the client (usually HTTP/1.1) */
			Request(Connection & connection, const std::string & method, const std::string & url, const std::string & protocol_and_version)
				: connection_(connection),
				  method_(method),
				  url_(url),
				  protocol_and_version_(protocol_and_version)
			{ /* no-op */ }

			Connection & connection_;			///< the connection to use to respond to the request
			std::string method_;				///< the method of the request, as supplied by the client
			std::string url_;					///< the location of the resource requested
			std::string protocol_and_version_;	///< the protocol and version used by the client (usually HTTP/1.1)
			HeaderFields header_fields_;		///< all header fields found while parsing the request
			std::vector< char > body_;			///< the body of the request
		};
	}
}

#endif
