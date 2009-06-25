#include "HTTPDataHandler.h"
#include <cassert>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <Acari/ParsingHelpers.hpp>
#include "HTTPRequestHandler.h"
#include "../Connection.h"
#include "../Details/Request.h"
#include "../Exceptions/HTTP.h"

using namespace Acari;

namespace Spin
{
	namespace Handlers
	{
		/*static */unsigned long HTTPDataHandler::attribute_index__(0xFFFFFFFF);
		/*static */boost::once_flag HTTPDataHandler::once_flag__(BOOST_ONCE_INIT);
		/*static */std::map< std::string, HTTPDataHandler::Method > HTTPDataHandler::supported_methods__;

		HTTPDataHandler::HTTPDataHandler(HTTPRequestHandler & request_handler)
			: request_handler_(request_handler)
		{
			boost::call_once(initStaticMembers, once_flag__);
		}

		/*virtual */void HTTPDataHandler::onDataReady(boost::shared_ptr< Connection > connection)
		{
			// get any pending data from the connection
			std::vector< char > buffer;
			std::size_t bytes_read;
			int reason;
			boost::tie( bytes_read, reason ) = connection->read(buffer);
			buffer.resize(bytes_read);

			/* Regardless of whether we have all the data pending on the 
			 * connection or not, we may have enough to get a request. In any 
			 * case, buffer now contains anything we have. */
			/* If the connection has an attribute, it's the request with any data
			 * that was left over from the last time. */
			boost::any & connection_attribute(connection->getAttribute(attribute_index__));
			boost::shared_ptr< Details::Request > request;
			std::vector< char >::iterator where;
			bool end_of_headers_found(false);
after_connection_attributes_are_got:
			do 
			{
				if (connection_attribute.empty() /* no existing attribute */)
				{	// parse the request, as it is new
					boost::tie(request, where) = extractRequestHeader< Connection, Details::Request, Exceptions::HTTP::UnknownMethod, Exceptions::HTTP::UnsupportedProtocol >(connection, supported_methods__, buffer.begin(), buffer.end());
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
						boost::tie(header.name_, header.value_) = splitHeader< Exceptions::HTTP::InvalidHeader >(where, whence);
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

