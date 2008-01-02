#include "HTTPDataHandler.h"
#include <cassert>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include "HTTPRequestHandler.h"
#include "../Connection.h"
#include "../Details/Request.h"
#include "../Exceptions/HTTP.h"

namespace Spin
{
	namespace Handlers
	{
		namespace
		{
			bool isIgnorableWhiteSpace(char c)
			{
				return c == 9 || c == 10 || c == 13 || c == 32;
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
			bool moreHeaders(Iterator where, const Iterator & whence)
			{
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
			Iterator backupThroughNewlines(Iterator begin, Iterator where)
			{
				while ((where != begin) && ((where - 1) != begin) && isNewLine(*(where - 1)))
					--where;
				return where;
			}

			template < typename Iterator >
			Iterator findHeaderEnd(Iterator from_where, Iterator where, const Iterator & whence)
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

				/* Now, if we're at a non-whitespace character, we're at the end of the header.
				 * If we're at a whitespace character, we are not at the end of the header and 
				 * we can keep on looking. If we're at the end of the buffer, we need to do a bit
				 * more pondering to see whether we know if we're at the end of the header or not... */
				if (where != whence && !isIgnorableWhiteSpace(*where))
				{
					/* In order to know whether this was the last header, we need to count the
					 * number of newline characters at the end of the header field, for which
					 * we have to back up through the newline characters. */
					return backupThroughNewlines(from_where, where);
				}
				else if (where != whence && isIgnorableWhiteSpace(*where)) // keep looking
					return findHeaderEnd(from_where, where, whence);
				else
				{
					assert(where == whence);
					/* We're at the end of the buffer. There is only one way to know whether or not 
					 * we are at the end of the header: we are *certainly* at the end of the header
					 * if we're at the end of all headers. Otherwise, we don't know whether we're at 
					 * the end of the header or not, as the next line might start with ignorable 
					 * white space. */
					where = backupThroughNewlines(from_where, where);
					if (!moreHeaders(where, whence))
					{
						/* there are no more headers to come, so "where" is at the end of the current header */
						return where;
					}
					else
					{ /* there might be more headers to come, and therefore there might be a line to 
					   * come that starts with ignorable white space, which would make it part of the 
					   * current header. Bummer, but we'll have to return from_where. */
						return from_where;
					}
				}
			}

			template < typename Iterator >
			Iterator findHeaderEnd(Iterator where, const Iterator & whence)
			{
				return findHeaderEnd(where, where, whence);
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
				if (colon == end) throw Exceptions::HTTP::InvalidHeader(begin, end);
				std::string name(chomp(begin, colon));
				++colon;
				std::string value(chomp(colon, end));
				return std::make_pair(name, value);
			}

			template < typename Iterator, typename Method >
			std::pair< boost::shared_ptr< Details::Request >, Iterator > extractHeader(Connection & connection, const std::map< std::string, Method > & supported_methods, Iterator curr, const Iterator & end)
			{
				Iterator begin(curr);
				/* The first line of an HTTP request consists of the method, the URL of 
				 * whatever is requested and the protocol and version. Some broken 
				 * implementations may send white spaces (characters 9, 10, 13 or 32) before
				 * sending the request method, so we will skip those if need be. */
				std::vector< char >::iterator where(curr);
				// ignore leading white space
				where = advanceThroughIgnorableWhiteSpace(where, end);
				std::vector< char >::iterator whence(where);
				whence = advanceUntilIgnorableWhiteSpace(whence, end);
				// if we're at the end of the buffer already, we do not have the entire method string.
				if (whence == end)
					return std::make_pair(boost::shared_ptr< Details::Request >(), begin);
				else
				{ /* carry on */ }
				// the request method is now in [where,whence)
				std::string method(where, whence);
				if (supported_methods.find(method) == supported_methods.end())
					throw Exceptions::HTTP::UnknownMethod(where, whence);
				else
				{ /* carry on to get the URL */ }
				where = advanceThroughIgnorableWhiteSpace(whence, end);
				whence = advanceUntilIgnorableWhiteSpace(where, end);
				// if we're at the end of the buffer, we do not have the entire URL
				if (whence == end)
					return std::make_pair(boost::shared_ptr< Details::Request >(), begin);
				else
				{ /* carry on */ }
				// the URL is now in [where, whence)
				std::string url(where, whence);
				where = advanceThroughIgnorableWhiteSpace(whence, end);
				whence = advanceUntilIgnorableWhiteSpace(where, end);
				// if we're at the end of the buffer, we do not have the entire protocol and version
				if (whence == end)
					return std::make_pair(boost::shared_ptr< Details::Request >(), begin);
				else
				{ /* carry on */ }
				// the protocol and version is now in [where, whence)
				std::string protocol_and_version(where, whence);
				// there are only two protocol strings we support: HTTP/1.0 and HTTP/1.1
				if (protocol_and_version != "HTTP/1.0" && protocol_and_version != "HTTP/1.1")
					throw Exceptions::HTTP::UnsupportedProtocol(protocol_and_version.begin(), protocol_and_version.end());
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

		/*virtual */void HTTPDataHandler::onDataReady(Connection & connection) const
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
			bool end_of_headers_found(false);
after_connection_attributes_are_got:
			do 
			{
				if (connection_attribute.empty() /* no existing attribute */)
				{	// parse the request, as it is new
					boost::tie(request, where) = extractHeader(connection, supported_methods__, buffer.begin(), buffer.end());
					if (!request && where == buffer.begin())
					{
						/* the extraction errored out because the first line was incomplete. 
						 * If this is the case, we will store whatever we got in the connection 
						 * attributes and return. */
						break;
					}
					else
					{ /* all is well */ }
					assert(request || where == buffer.end());
				}
				else
				{
					std::vector< char > temp_buffer;
					boost::tie( request, temp_buffer, end_of_headers_found ) = boost::any_cast< boost::tuple< boost::shared_ptr< Details::Request >, std::vector< char >, bool > >(connection_attribute);
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
			bool end_of_buffer_found(false);
			if (request) do
			{
				std::vector< char >::iterator whence(where);
				whence = advanceThroughIgnorableWhiteSpace(whence, buffer.end());
				if (moreHeaders(where, whence))
				{
					where = whence;
					whence = findHeaderEnd(where, buffer.end());
					if (where != whence)
					{
						Details::Header header;
						boost::tie(header.name_, header.value_) = splitHeader(where, whence);
						request->header_fields_.push_back(header);
						where = whence;
					}
					else
					{	/* we're at the end of the current buffer, but more headers are still
						 * to come. We'll break out of the loop (by setting end_of_buffer_found, 
						 * in some corner-cases, where will not be at buffer.end() but we will 
						 * still have found the end of the buffer) and store what we currently know
						 * about the request in the connection attributes (the end of the headers
						 * will not have been found, so this will happen automatically) */
						end_of_buffer_found = true;
					}
				}
				else // we have all of the header
				{
					where = whence;
					end_of_headers_found = true;
				}
			} while (!end_of_headers_found && where != buffer.end() && !end_of_buffer_found);
			if (end_of_headers_found)
			{
				/* When we get here, we don't know whether the request has a body.
				 * If it does, there is a Content-Length header among the headers
				 * that will contain the size of the body. */
				Details::HeaderFields::const_iterator curr(request->header_fields_.begin());
				Details::HeaderFields::const_iterator end(request->header_fields_.end());
				while (curr != end && curr->name_ != "Content-Length") ++curr;
				bool complete_body_found(false);
				if (curr != end)
				{
					std::size_t body_size(boost::lexical_cast< std::size_t >(curr->value_));
					std::vector< char >::iterator whence(where);
					/* When we get here, the "where" and "whence" iterators should both be 
					 * at the start of the body. We will advance "whence" to the end of the
					 * body - which should be within the bounds of the buffer. */
					if (std::size_t(std::distance(whence, buffer.end())) < body_size)
					{
						connection_attribute = boost::make_tuple(request, std::vector< char >(where, buffer.end()), end_of_headers_found);
					}
					else
					{
						std::advance(whence, body_size);
						assert(request->body_.empty());
						request->body_.insert(request->body_.end(), where, whence);
						where = whence;
						complete_body_found = true;
					}
				}
				else
				{ /* request does not have a body */
					complete_body_found = true;
				}
				if (complete_body_found)
				{
					request_handler_.handle(request);
					/* Now, if whence is not at the end of the buffer, more requests may be 
					 * in the buffer. We need to handle those. */
					if (where != buffer.end())
					{
						connection_attribute = boost::make_tuple(boost::shared_ptr< Details::Request >(), std::vector< char >(where, buffer.end()), false);
						buffer.clear();
						goto after_connection_attributes_are_got;
					}
					else
					{ /* done */ }
				}
			}
			else
			{
				connection_attribute = boost::make_tuple(request, std::vector< char >(where, buffer.end()), end_of_headers_found);
			}
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

