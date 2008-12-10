#include "Request.h"
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/thread/once.hpp>
#include <Acari/ParsingHelpers.hpp>
#include <Acari/urlencode.h>
#include <Scorpion/Context.h>
#include <Spin/Connection.h>
#include <Spin/Connector.h>
#include <Spin/Exceptions/HTTP.h>
#include "Private/parseURL.h"
#include "Session.h"

using namespace Acari;

namespace Damon
{
	namespace
	{
		template < typename C >
		struct ValidateURL_
		{
			typedef C argument_type;

			ValidateURL_(const std::string & server, unsigned short port, std::string C::* p)
				: server_(server),
				  port_(port),
				  p_(p)
			{ /* no-op */ }

			bool operator()(const C & c) const
			{
				using Private::parseURL;

				std::string protocol;
				std::string server;
				boost::uint16_t port;
				std::string resource;
				boost::tie(protocol, server, port, resource) = parseURL(c.*p_);

				return (server == server_) && (port_ == port);
			}

			std::string server_;
			unsigned short port_;
			std::string C::* p_;
		};

		template < typename C >
		ValidateURL_< C > ValidateURL(const std::string & server, unsigned short port, std::string C::* p)
		{
			return ValidateURL_< C >(server, port, p);
		}
	}

	static unsigned long attribute_index__(0xFFFFFFFF);
	static boost::once_flag once_flag__(BOOST_ONCE_INIT);

	void initAttributeIndex()
	{
		assert(attribute_index__ == 0xFFFFFFFF);
		attribute_index__ = Spin::Connection::allocateAttribute();
	}

	Request::Request(const std::string & url, Method method/* = get__*/)
		: url_(url),
		  method_(method)
	{ /* no-op */ }

	Request::~Request()
	{ /* no-op */ }

	Response getResponse(boost::shared_ptr< Spin::Connection > connection)
	{
		boost::call_once(initAttributeIndex, once_flag__);
		bool retrying(false);
retry:
		/* HERE we should make the connection blocking in a scoped manner if it isn't already */

		std::vector< char > buffer;

		/* When we get here, we may have some data in the connection's 
		 * attributes already, which we can get now */
		boost::any & connection_attribute(connection->getAttribute(attribute_index__));
		std::vector< char >::iterator where;
		bool end_of_headers_found(false);
		bool end_of_buffer_found(false);
		boost::shared_ptr< Response > response;
		do 
		{
			if (connection_attribute.empty() /* no existing attribute */)
			{	// parse the request, as it is new
				if (retrying || end_of_buffer_found || buffer.empty() || connection->poll())
				{
					std::vector< char > tmp_buffer(buffer);
					buffer.clear();
					std::pair< std::size_t, int > result(connection->read(buffer));
					switch (result.second /* reason */)		// this is a switch mainly for documentation purposes - it could have been an if, but I think this is clearer
					{
					case Spin::Connection::no_error__ :
					case Spin::Connection::should_retry__ :
					case Spin::Connection::should_read__ :
						/* in any of these cases, we either have everything we came for or we 
						 * should read some more. We might as well check whether we have 
						 * everything we can for and retry only after that, because we will 
						 * probably have either the complete response or have to allow the 
						 * server a bit of time to push the rest of the data through. */
						break;
					default :
						throw std::runtime_error("Unexpected connection state"); // be more eloquent HERE
					}
					const std::size_t & size(result.first);
					buffer.resize(size);
					buffer.insert(buffer.begin(), tmp_buffer.begin(), tmp_buffer.end());
				}
				else
				{ /* already have data to work with */ }
				boost::tie(response, where) = extractResponseHeader< Spin::Connection, Response, Spin::Exceptions::HTTP::UnsupportedProtocol >(connection, buffer.begin(), buffer.end());
				if (!response && where == buffer.begin())
				{
					/* the extraction errored out because the first line was incomplete. 
					 * If this is the case, we will store whatever we got in the connection 
					 * attributes and return. */
					break;
				}
				else
				{ /* all is well */ }
				assert(response || where == buffer.end());
			}
			else
			{
				std::vector< char > temp_buffer;
				boost::tie( response, temp_buffer, end_of_headers_found, end_of_buffer_found ) = boost::any_cast< boost::tuple< boost::shared_ptr< Response >, std::vector< char >, bool, bool > >(connection_attribute);
				buffer.insert(buffer.begin(), temp_buffer.begin(), temp_buffer.end());
				/* connection_attribute.clear(); ==> */ boost::any a; connection_attribute.swap(a);
				where = buffer.begin();
			}
		} while(!(response || where == buffer.end()));
		/* from here on, white space is important as we need to count the number of 
		 * carriage returns (13) or newlines (10). If we find two of them before 
		 * finding a non-whitespace character (that is: two newlines or two carriage 
		 * returns), the requests consists of only the header. If we find only one, 
		 * there is a header before the end of the response, and there might be a 
		 * body. */
		end_of_buffer_found = false;
		if (response) do
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
					boost::tie(header.name_, header.value_) = splitHeader< Spin::Exceptions::HTTP::InvalidHeader >(where, whence);
					response->header_fields_.push_back(header);
					where = whence;
				}
				else
				{	/* we're at the end of the current buffer, but more headers are still
					 * to come. We'll break out of the loop (by setting end_of_buffer_found, 
					 * in some corner-cases, where will not be at buffer.end() but we will 
					 * still have found the end of the buffer) and store what we currently know
					 * about the response in the connection attributes (the end of the headers
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
			/* When we get here, we don't know whether the response has a body.
			 * If it does, there is a Content-Length header among the headers
			 * that will contain the size of the body. */
			Details::HeaderFields::const_iterator curr(response->header_fields_.begin());
			Details::HeaderFields::const_iterator end(response->header_fields_.end());
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
					connection_attribute = boost::make_tuple(response, std::vector< char >(where, buffer.end()), end_of_headers_found);
				}
				else
				{
					std::advance(whence, body_size);
					assert(response->body_.empty());
					response->body_.insert(response->body_.end(), where, whence);
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
				/* Now, if whence is not at the end of the buffer, more requests may be 
				 * in the buffer. We need to handle those. */
				if (where != buffer.end())
				{
					connection_attribute = boost::make_tuple(boost::shared_ptr< Response >(), std::vector< char >(where, buffer.end()), false, end_of_buffer_found);
				}
				else
				{ /* done */ }
			}
		}
		else
		{
			connection_attribute = boost::make_tuple(response, std::vector< char >(where, buffer.end()), end_of_headers_found, end_of_buffer_found);
			retrying = true; // force a read on the connection
			goto retry; // tail recursion
		}

		return Response(*response);
	}

	/*DAMON_API */Response send(Session & session, const Request & request)
	{
		using Private::parseURL;

		std::string protocol;
		std::string server;
		boost::uint16_t port;
		std::string resource;
		boost::tie(protocol, server, port, resource) = parseURL(request.url_);
		resource = urlencode(resource);
		bool secured(protocol == "https");

		if (secured)
		{
			if (!session.context_)
			{
				session.context_ = new Scorpion::Context;
			}
			else
			{ /* already have a context */ }
		}
		else
		{ /* no security context required */ }
		assert((secured && session.context_) || !secured);

		/* the part of the URL that's important for the connection cache is 
		* the server and the port. The rest of the URL (i.e. protocol and 
		* resource) isn't important as it doesn't affect with whom we talk
		* but rather how (which should not change from one call to the same
		* port to another) and with which resource (which may change, but
		* we don't care about that). */
		boost::format cache_key("%1%:%2%");
		cache_key
			% server
			% port
			;
		Session::ConnectionCache_::iterator cached_connection(session.connection_cache_.find(cache_key.str()));
		do 
		{
			if (cached_connection == session.connection_cache_.end())
			{
				cached_connection = session.connection_cache_.insert(
					Session::ConnectionCache_::value_type(
					cache_key.str(),
						(
							secured
								? Spin::Connector::getInstance().connect(*session.context_, server, port)
								: Spin::Connector::getInstance().connect(server, port)
							)
						)
					).first;
			}
			else
			{ /* we've found the connection */ }
			assert(cached_connection != session.connection_cache_.end());
			if (cached_connection->second->getStatus() != Spin::Connection::good__)
			{
				session.connection_cache_.erase(cached_connection);
				cached_connection = session.connection_cache_.end();
			}
			else
			{ /* all is well */ }
		} while(cached_connection == session.connection_cache_.end());

		cached_connection->second->write(request);

		return getResponse(cached_connection->second);
	}

	/*DAMON_API */Response send(const Request & request)
	{
		Session session;

		return send(session, request);
	}

	/*DAMON_API */std::vector< Response > send(Session & session, const std::vector< Request > & requests)
	{
		using Private::parseURL;

		if (requests.empty())
			return std::vector< Response >();
		else
		{ /* carry on */ }

		std::string protocol;
		std::string server;
		boost::uint16_t port;
		std::string resource;
		boost::tie(protocol, server, port, resource) = parseURL(requests[0].url_);
		bool secured(protocol == "https");

		if (secured)
		{
			if (!session.context_)
			{
				session.context_ = new Scorpion::Context;
			}
			else
			{ /* already have a context */ }
		}
		else
		{ /* no security context required */ }
		assert((secured && session.context_) || !secured);

		/* the part of the URL that's important for the connection cache is 
		 * the server and the port. The rest of the URL (i.e. protocol and 
		 * resource) isn't important as it doesn't affect with whom we talk
		 * but rather how (which should not change from one call to the same
		 * port to another) and with which resource (which may change, but
		 * we don't care about that). */
		boost::format cache_key("%1%:%2%");
		cache_key
			% server
			% port
			;
		Session::ConnectionCache_::iterator cached_connection(session.connection_cache_.find(cache_key.str()));
		do 
		{
			if (cached_connection == session.connection_cache_.end())
			{
				cached_connection = session.connection_cache_.insert(
						Session::ConnectionCache_::value_type(
							cache_key.str(),
							(
								secured
									? Spin::Connector::getInstance().connect(*session.context_, server, port)
									: Spin::Connector::getInstance().connect(server, port)
								)
							)
						).first;
			}
			else
			{ /* we've found the connection */ }
			assert(cached_connection != session.connection_cache_.end());
			if (cached_connection->second->getStatus() != Spin::Connection::good__)
			{
				session.connection_cache_.erase(cached_connection);
				cached_connection = session.connection_cache_.end();
			}
			else
			{ /* all is well */ }
		} while(cached_connection == session.connection_cache_.end());

		// validate that all of the requests use the same server and port parts in the URL
		assert(std::find_if(requests.begin(), requests.end(), std::not1(ValidateURL(server, port, &Request::url_))) == requests.end());

		std::for_each(requests.begin(), requests.end(), boost::bind(&Spin::Connection::write< Request >, cached_connection->second.get(), _1));

		std::vector< Response > responses;
		for (std::vector< Request >::size_type i(0); i < requests.size(); ++i)
			responses.push_back(getResponse(cached_connection->second));
		return responses;
	}

	/*DAMON_API */std::vector< Response > send(const std::vector< Request > & requests)
	{
		Session session;

		return send(session, requests);
	}

	namespace
	{
		struct AppendHeader
		{
			AppendHeader(std::string & str)
				: str_(str)
			{ /* no-op */ }

			AppendHeader & operator()(const Details::Header & header)
			{
				boost::format fmt("%1%: %2%\r\n");
				fmt % header.name_ % header.value_;
				str_ += fmt.str();
				return *this;
			}

			std::string & str_;
		};
	}

	/*DAMON_API */std::string serialize(Request request)
	{
		using Private::parseURL;
		using Private::extractHost;

		std::string protocol;
		std::string server;
		boost::uint16_t port;
		std::string resource;
		boost::tie(protocol, server, port, resource) = parseURL(request.url_);
		resource = urlencode(resource);
		std::string retval;
		switch (request.method_)
		{
		case Request::options__ :
			retval = "OPTIONS ";
			break;
		case Request::get__ :
			retval = "GET ";
			break;
		case Request::head__ :
			retval = "HEAD ";
			break;
		case Request::post__ :
			retval = "POST ";
			break;
		case Request::put__ :
			retval = "PUT ";
			break;
		case Request::delete__ :
			retval = "DELETE ";
			break;
		case Request::trace__ :
			retval = "TRACE ";
			break;
		case Request::connect__ :
			retval = "CONNECT ";
			break;
		default :
			throw std::runtime_error("Unknown method");    // be more eloquent HERE
		}
		retval += resource;
		retval += " HTTP/1.1\r\n";

		if (std::find_if(request.header_fields_.begin(), request.header_fields_.end(), bind(&Details::Header::name_, _1) == "Host") == request.header_fields_.end())
			request.header_fields_.push_back(Details::Header("Host", extractHost(request.url_)));
		else
		{ /* Already has a Host header */ }
		if (std::find_if(request.header_fields_.begin(), request.header_fields_.end(), bind(&Details::Header::name_, _1) == "Content-Length") == request.header_fields_.end())
			request.header_fields_.push_back(Details::Header("Content-Length", boost::lexical_cast< std::string >(request.body_.size())));
		else
		{ /* Already has a Content-Length header */ }
		std::for_each(request.header_fields_.begin(), request.header_fields_.end(), AppendHeader(retval));
		retval += "\r\n";
		retval.insert(retval.end(), request.body_.begin(), request.body_.end());
	
		return retval;
	}
}
