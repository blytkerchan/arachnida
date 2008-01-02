#include "Response.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

namespace Spin
{
	namespace Details
	{
		/*static */const char * Response::reason_strings__[Response::_upper_bound__] =
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

			/*100*/"Continue",
			/*101*/"Switching Protocols",
			      0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

			/*200*/"OK",
			/*201*/"Created",
			/*202*/"Accepted",
			/*203*/"Non-Authoritative Information",
			/*204*/"No Content",
			/*205*/"Reset Content",
			/*206*/"Partial Content",
			                     0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

			/*300*/"Multiple Choices",
			/*301*/"Moved Permanently",
			/*302*/"Found",
			/*303*/"See Other",
			/*304*/"Not Modified",
			/*305*/"Use Proxy",
			/*306*/"Temporary Redirect",
			                     0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			
			/*400*/"Bad Request",
			/*401*/"Unauthorized", 
			/*402*/"Payment Required", 
			/*403*/"Forbidden",
			/*404*/"Not Found", 
			/*405*/"Method Not Allowed",
			/*406*/"Not Acceptable", 
			/*407*/"Proxy Authentication Required", 
			/*408*/"Request Time-out", 
			/*409*/"Conflict", 

			/*410*/"Gone",
			/*411*/"Length Required",
			/*412*/"Precondition Failed",
			/*413*/"Request Entity Too Large",
			/*414*/"Request-URI Too Large",
			/*415*/"Unsupported Media Type",
			/*416*/"Requested range not satisfiable",
			/*417*/"Expectation Failed",
			                        0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			/*500*/"Internal Server Error",
			/*501*/"Not Implemented",
			/*502*/"Bad Gateway",
			/*503*/"Service Unavailable",
			/*504*/"Gateway Time-out",
			/*505*/"HTTP Version not supported"
		};

		Response::Response(const std::string & protocol_and_version, StatusCode status_code)
			: protocol_and_version_(protocol_and_version),
			  status_code_(status_code),
			  reason_string_(reason_strings__[status_code])
		{ /* no-op */ }

		Response::Response(const std::string & protocol_and_version, int status_code, const std::string & reason_string)
			: protocol_and_version_(protocol_and_version),
			  status_code_(status_code),
			  reason_string_(reason_string)
		{ /* no-op */ }

		std::vector< char > serialize(const Response & response)
		{
			struct HeaderAppender
			{
				HeaderAppender(std::string & headers)
					: headers_(headers)
				{ /* no-op */ }

				HeaderAppender & operator()(const Header & header)
				{
					boost::format fmt("%1%: %2%\r\n");

					fmt
						% header.name_
						% header.value_
						;
					headers_ += fmt.str();

					return *this;
				}

				std::string & headers_;
			};

			using namespace boost::lambda;
			HeaderFields header_fields(response.header_fields_);
			if (std::find_if(header_fields.begin(), header_fields.end(), bind(&Header::name_, _1) == "Content-Length") == header_fields.end())
				header_fields.push_back(Header("Content-Length", boost::lexical_cast< std::string >(response.body_.size())));
			else
			{ /* Already has a Content-Length header */ }
			boost::format response_status_line("%1% %2% %3%\r\n");
			response_status_line
				% response.protocol_and_version_
				% response.status_code_
				% response.reason_string_
				;
			std::string headers(response_status_line.str());
			std::for_each(header_fields.begin(), header_fields.end(), HeaderAppender(headers));
			headers += "\r\n";

			std::vector< char > retval(headers.begin(), headers.end());
			retval.insert(retval.end(), response.body_.begin(), response.body_.end());
			return retval;
		}
	}
}

