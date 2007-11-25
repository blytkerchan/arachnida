#include "HTTPDataHandler.h"
#include <cassert>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include "../Connection.h"
#include "../Details/Request.h"

namespace Spin
{
	namespace Handlers
	{
		namespace
		{
			bool isIgnorableWhiteSpace(char c)
			{
				return c == 8 || c == 10 || c == 13 || c == 32;
			}

			std::string ltrim(const std::string & s)
			{
				std::string::const_iterator where(s.begin());
				while (where != s.end() && isIgnorableWhiteSpace(*where))
					++where;
				return std::string(where, s.end());
			}

			std::string rtrim(std::string s)
			{
				std::string::reverse_iterator where(s.rbegin());
				while (where != s.rend() && isIgnorableWhiteSpace(*where))
					++where;
				s.erase(std::distance(where, s.rend()));
				return s;
			}

			bool isNewLine(char c)
			{
				return c == 10 || c == 13;
			}

			template < typename Iterator >
			Iterator advanceThroughIgnorableWhiteSpace(Iterator where, const Iterator & whence)
			{
				while (where != whence && isIgnorableWhiteSpace(*where))
					++where;
				return where;
			}

			template < typename Iterator >
			Iterator advanceUntilIgnorableWhiteSpace(Iterator where, const Iterator & whence)
			{
				while (where != whence && !isIgnorableWhiteSpace(*where))
					++where;
				return where;
			}

			template < typename Iterator >
			Iterator findHeaderEnd(Iterator where, const Iterator & whence)
			{
				/* The end of a header in HTTP is at a new line followed by a non-whitespace 
				 * character or the end of the buffer. If the newline is followed by a 
				 * non-newline whitespace character, the header value continues on the next 
				 * line. */

				while (where != whence && !isNewLine(*where))
					++where;
				// at the newline or at the end of the buffer. Advance through the newlines
				while (where != whence && isNewLine(*where))
					++where;
				// now, if we're at the end of the buffer or at a non-whitespace character, we're at the end of the header
				if (where == whence || !isIgnorableWhiteSpace(*where))
					return where;
				else // keep looking
					return findHeaderEnd(where, whence);
			}

			template < typename Iterator >
			bool moreHeaders(Iterator where, const Iterator & whence)
			{
				if (where == whence) return false;
				unsigned long cr(0);
				unsigned long lf(0);
				assert(std::distance(where, whence) < 0xFFFFFFFF); // fits in an unsigned long
				for ( ; where != whence; ++where)
				{
					if (*where == 13) ++cr;
					if (*where == 10) ++lf;
				}
				return !(cr >= 2 || lf >= 2);
			}

			template < typename Iterator >
			std::string chomp(Iterator begin, Iterator end)
			{
				while (begin != end && isIgnorableWhiteSpace(*begin)) ++begin;
				while (begin != end && isIgnorableWhiteSpace(*(end - 1))) --end;
				return std::string(begin, end);
			}

			template < typename Iterator >
			std::pair< std::string, std::string > splitHeader(Iterator begin, const Iterator & end)
			{
				Iterator colon(std::find(begin, end, ':'));
				if (colon == end) throw std::runtime_error("Invalid header field");
				std::string name(chomp(begin, colon));
				++colon;
				if (colon == end) throw std::runtime_error("Empty header field");
				std::string value(chomp(colon, end));
				return std::make_pair(name, value);
			}

			template < typename Iterator, typename Method >
			std::pair< boost::shared_ptr< Details::Request >, Iterator > extractHeader(Connection & connection, const std::map< std::string, Method > & supported_methods, Iterator curr, const Iterator & end)
			{
				/* The first line of an HTTP request consists of the method, the URL of 
				 * whatever is requested and the protocol and version. Some broken 
				 * implementations may send white spaces (characters 10, 13 or 32) before
				 * sending the request method, so we will skip those if need be. */
				std::vector< char >::iterator where(curr);
				// ignore leading white space
				where = advanceThroughIgnorableWhiteSpace(where, end);
				std::vector< char >::iterator whence(where);
				whence = advanceUntilIgnorableWhiteSpace(whence, end);
				// the request method is now in [where,whence)
				std::string method(where, whence);
				if (supported_methods.find(method) == supported_methods.end())
					throw std::runtime_error("Unknown method");
				else
				{ /* carry on to get the URL */ }
				where = advanceThroughIgnorableWhiteSpace(whence, end);
				whence = advanceUntilIgnorableWhiteSpace(where, end);
				// the URL is now in [where, whence)
				std::string url(where, whence);
				where = advanceThroughIgnorableWhiteSpace(whence, end);
				whence = advanceUntilIgnorableWhiteSpace(where, end);
				// the protocol and version is now in [where, whence)
				std::string protocol_and_version(where, whence);
				// there are only two protocol strings we support: HTTP/1.0 and HTTP/1.1
				if (protocol_and_version != "HTTP/1.0" && protocol_and_version != "HTTP/1.1")
					throw std::runtime_error("Unsupported protocol");
				else
				{ /* carry on */ }

				return std::make_pair(boost::shared_ptr< Details::Request >(new Details::Request(connection, method, url, protocol_and_version)), whence);
			}
		}

		/*static */unsigned long HTTPDataHandler::attribute_index__(0xFFFFFFFF);
		/*static */boost::once_flag HTTPDataHandler::once_flag__(BOOST_ONCE_INIT);
		/*static */std::map< std::string, HTTPDataHandler::Method > HTTPDataHandler::supported_methods__;

		HTTPDataHandler::HTTPDataHandler(HTTPRequestHandler & request_handler)
			: request_handler_(request_handler)
		{
			boost::call_once(initStaticMembers, once_flag__);
		}

		/*virtual */void HTTPDataHandler::onDataReady(Connection & connection) const/* = 0*/
		{
			// get any pending data from the connection
			std::vector< char > buffer;
			std::size_t bytes_read;
			int reason;
			boost::tie( bytes_read, reason ) = connection.read(buffer);
			buffer.resize(bytes_read);

			/* Regardless of whether we have all the data pending on the 
			 * connection or not, we may have enough to get a request. In any 
			 * case, buffer now contains anything we have. */
			/* If the connection has an attribute, it's the request with any data
			 * that was left over from the last time. */
			boost::any & connection_attribute(connection.getAttribute(attribute_index__));
			boost::shared_ptr< Details::Request > request;
			std::vector< char >::iterator where;
			do 
			{
				if (connection_attribute.empty() /* no existing attribute */)
				{	// parse the request, as it is new
					boost::tie(request, where) = extractHeader(connection, supported_methods__, buffer.begin(), buffer.end());
					assert(request || where == buffer.end());
				}
				else
				{
					std::vector< char > temp_buffer;
					boost::tie( request, temp_buffer ) = boost::any_cast< std::pair< boost::shared_ptr< Details::Request >, std::vector< char > > >(connection_attribute);
					buffer.insert(buffer.begin(), temp_buffer.begin(), temp_buffer.end());
					/* connection_attribute.clear(); ==> */ boost::any a; connection_attribute.swap(a);
					where = buffer.begin();
				}
			} while(!(request || where == buffer.end()));
			/* from here on, white space is important as we need to count the number of 
			 * carriage returns (13) or newlines (10). If we find two of them before 
			 * finding a non-whitespace character (that is: two newlines or two carriage 
			 * returns), the requests consists of only the header. If we find only one, 
			 * there is a header before the end of the request, and there might be a 
			 * body. */
			bool end_of_headers_found(false);
			do
			{
				std::vector< char >::iterator whence(where);
				whence = advanceThroughIgnorableWhiteSpace(whence, buffer.end());
				if (moreHeaders(where, whence))
				{
					where = whence;
					whence = findHeaderEnd(where, buffer.end());
					Details::Request::Header header;
					boost::tie(header.name_, header.value_) = splitHeader(where, whence);
					request->header_fields_.push_back(header);
					where = whence;
				}
				else // we have all of the header
					end_of_headers_found = true;
			} while (!end_of_headers_found && where != buffer.end());
			if (end_of_headers_found)
			{
				/* When we get here, we don't know whether the request has a body.
				 * If it does, there is a Content-Length header among the headers
				 * that will contain the size of the body. */

			}
			else
			{ /* HERE!! store whatever we have in the connection attributes */ }
		}

		/*static */void HTTPDataHandler::initStaticMembers()
		{
			attribute_index__ = Connection::allocateAttribute();
			supported_methods__.insert(std::map< std::string, Method >::value_type("OPTIONS", options__));
			supported_methods__.insert(std::map< std::string, Method >::value_type("GET", get__));
			supported_methods__.insert(std::map< std::string, Method >::value_type("HEAD", head__));
			supported_methods__.insert(std::map< std::string, Method >::value_type("POST", post__));
			supported_methods__.insert(std::map< std::string, Method >::value_type("PUT", put__));
			supported_methods__.insert(std::map< std::string, Method >::value_type("DELETE", delete__));
			supported_methods__.insert(std::map< std::string, Method >::value_type("TRACE", trace__));
			supported_methods__.insert(std::map< std::string, Method >::value_type("CONNECT", connect__));
		}
	}
}

