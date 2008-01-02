#ifndef _spin_details_response_h
#define _spin_details_response_h

#include <string>
#include <vector>
#include "prologue.h"
#include "Header.h"

namespace Spin
{
	namespace Details
	{
		struct SPIN_API Response
		{
			enum StatusCode
			{
				continue__ = 100,							///< Section 10.1.1: Continue
				switching_protocols__ = 101,				///< Section 10.1.2: Switching Protocols
				ok__ = 200,									///< Section 10.2.1: OK
				created__ = 201,							///< Section 10.2.2: Created
				accepted__ = 202,							///< Section 10.2.3: Accepted
				non_authoritive_information__ = 203,		///< Section 10.2.4: Non-Authoritative Information
				no_content__ = 204,							///< Section 10.2.5: No Content
				reset_content__ = 205,						///< Section 10.2.6: Reset Content
				partial_content__ = 206,					///< Section 10.2.7: Partial Content
				multiple_choices__ = 300,					///< Section 10.3.1: Multiple Choices
				moved_permanently__ = 301,					///< Section 10.3.2: Moved Permanently
				found__ = 302,								///< Section 10.3.3: Found
				see_other__ = 303,							///< Section 10.3.4: See Other
				not_modified__ = 304,						///< Section 10.3.5: Not Modified
				use_proxy__ = 305,							///< Section 10.3.6: Use Proxy
				temporary_redirect__ = 307,					///< Section 10.3.8: Temporary Redirect
				bad_request__ = 400,						///< Section 10.4.1: Bad Request
				unauthorized__ = 401,						///< Section 10.4.2: Unauthorized
				payment_required__ = 402,					///< Section 10.4.3: Payment Required
				forbidden__ = 403,							///< Section 10.4.4: Forbidden
				not_found__ = 404,							///< Section 10.4.5: Not Found
				method_not_allowed__ = 405,					///< Section 10.4.6: Method Not Allowed
				not_acceptable__ = 406,						///< Section 10.4.7: Not Acceptable
				proxy_authentication_required__ = 407,		///< Section 10.4.8: Proxy Authentication Required
				request_time_out__ = 408,					///< Section 10.4.9: Request Time-out
				conflict__ = 409,							///< Section 10.4.10: Conflict
				gone__ = 410,								///< Section 10.4.11: Gone
				length_required__ = 411,					///< Section 10.4.12: Length Required
				precondition_failed__ = 412,				///< Section 10.4.13: Precondition Failed
				request_entity_too_large__ = 413,			///< Section 10.4.14: Request Entity Too Large
				request_uri_too_large__ = 414,				///< Section 10.4.15: Request-URI Too Large
				unsupported_media_type__ = 415,				///< Section 10.4.16: Unsupported Media Type
				requested_change_not_satisfiable__ = 416,	///< Section 10.4.17: Requested range not satisfiable
				expectation_failed__ = 417,					///< Section 10.4.18: Expectation Failed
				internal_server_error__ = 500,				///< Section 10.5.1: Internal Server Error
				not_implemented__ = 501,					///< Section 10.5.2: Not Implemented
				bad_gateway__ = 502,						///< Section 10.5.3: Bad Gateway
				service_unavailable__ = 503,				///< Section 10.5.4: Service Unavailable
				gateway_time_out__ = 504,					///< Section 10.5.5: Gateway Time-out
				http_version_not_supported__ = 505,			///< Section 10.5.6: HTTP Version not supported
				_upper_bound__ = 506
			};

			Response(const std::string & protocol_and_version, StatusCode status_code);
			Response(const std::string & protocol_and_version, int status_code, const std::string & reason_string);

			void addHeader(const std::string & name, const std::string & value) { header_fields_.push_back(Header(name, value)); }
			Response & operator()(const std::string & name, const std::string & value) { addHeader(name, value); return *this; }

			void setBody(const std::vector< char > & body) { body_ = body; }
			Response & operator()(const std::vector< char > & body) { setBody(body); return *this; }
			Response & operator()(const std::string & body) { setBody(std::vector< char >(body.begin(), body.end())); return *this; }
			Response & operator()(const char * body) { return (*this)(std::string(body)); }
			template < typename T >
			Response & operator()(const T & body) { return (*this)(serialize(body)); }

			std::string protocol_and_version_;	///< the protocol and version of the original request (see Sections 6, 3.1 and 10.5.6)
			int status_code_;					///< the status code for the response
			std::string reason_string_;			///< the human-readable equivalent of the status code
			HeaderFields header_fields_;		///< all header fields found while parsing the request
			std::vector< char > body_;			///< the body of the request

		private :
			static const char * reason_strings__[_upper_bound__];
		};

		SPIN_API std::vector< char > serialize(const Response & response);
	}
}

#endif
