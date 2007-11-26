#ifndef _spin_details_request_h
#define _spin_details_request_h

#include <string>
#include <list>
#include <vector>
#include <utility>

namespace Spin
{
	class Connection;
	namespace Details
	{
		struct Request
		{
			struct Header
			{
				std::string name_;
				std::string value_;
			};
			typedef std::list< Header > HeaderFields;

			Request(Connection & connection, const std::string & method, const std::string & url, const std::string & protocol_and_version)
				: connection_(connection),
				  method_(method),
				  url_(url),
				  protocol_and_version_(protocol_and_version)
			{ /* no-op */ }

			Connection & connection_;
			std::string method_;
			std::string url_;
			std::string protocol_and_version_;
			HeaderFields header_fields_;
			std::vector< char > body_;
		};
	}
}

#endif