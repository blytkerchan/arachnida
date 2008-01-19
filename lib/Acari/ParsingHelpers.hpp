#ifndef _acari_parsinghelpers_hpp
#define _acari_parsinghelpers_hpp

#include <map>
#include <boost/lexical_cast.hpp>

namespace Acari
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
		Iterator advanceUntilEndOfLine(Iterator where, const Iterator & whence)
		{
			while (where != whence && !isNewLine(*where))
				++where;

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

		template < typename InvalidHeader, typename Iterator >
		std::pair< std::string, std::string > splitHeader(Iterator begin, const Iterator & end)
		{
			Iterator colon(std::find(begin, end, ':'));
			if (colon == end) throw InvalidHeader(begin, end);
			std::string name(chomp(begin, colon));
			++colon;
			std::string value(chomp(colon, end));
			return std::make_pair(name, value);
		}

		template < typename Connection, typename Request, typename UnknownMethod, typename UnsupportedProtocol, typename Iterator, typename Method >
		std::pair< boost::shared_ptr< Request >, Iterator > extractRequestHeader(boost::shared_ptr< Connection > connection, const std::map< std::string, Method > & supported_methods, Iterator curr, const Iterator & end)
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
				return std::make_pair(boost::shared_ptr< Request >(), begin);
			else
			{ /* carry on */ }
			// the request method is now in [where,whence)
			std::string method(where, whence);
			if (supported_methods.find(method) == supported_methods.end())
				throw UnknownMethod(where, whence);
			else
			{ /* carry on to get the URL */ }
			where = advanceThroughIgnorableWhiteSpace(whence, end);
			whence = advanceUntilIgnorableWhiteSpace(where, end);
			// if we're at the end of the buffer, we do not have the entire URL
			if (whence == end)
				return std::make_pair(boost::shared_ptr< Request >(), begin);
			else
			{ /* carry on */ }
			// the URL is now in [where, whence)
			std::string url(where, whence);
			where = advanceThroughIgnorableWhiteSpace(whence, end);
			whence = advanceUntilIgnorableWhiteSpace(where, end);
			// if we're at the end of the buffer, we do not have the entire protocol and version
			if (whence == end)
				return std::make_pair(boost::shared_ptr< Request >(), begin);
			else
			{ /* carry on */ }
			// the protocol and version is now in [where, whence)
			std::string protocol_and_version(where, whence);
			// there are only two protocol strings we support: HTTP/1.0 and HTTP/1.1
			if (protocol_and_version != "HTTP/1.0" && protocol_and_version != "HTTP/1.1")
				throw UnsupportedProtocol(protocol_and_version.begin(), protocol_and_version.end());
			else
			{ /* carry on */ }

			return std::make_pair(boost::shared_ptr< Request >(new Request(connection, method, url, protocol_and_version)), whence);
		}

		template < typename Connection, typename Response, typename UnsupportedProtocol, typename Iterator >
		std::pair< boost::shared_ptr< Response >, Iterator > extractResponseHeader(Connection & connection, Iterator curr, const Iterator & end)
		{
			Iterator begin(curr);
			/* The first line of an HTTP response consists of the protocol and version,
			 * the result code and a message intended for human consumption. Some broken 
			 * implementations may send white spaces (characters 9, 10, 13 or 32) before
			 * sending the response, so we will skip those if need be. */
			std::vector< char >::iterator where(curr);
			// ignore leading white space
			where = advanceThroughIgnorableWhiteSpace(where, end);
			std::vector< char >::iterator whence(where);
			whence = advanceUntilIgnorableWhiteSpace(whence, end);
			// if we're at the end of the buffer already, we do not have the entire method string.
			if (whence == end)
				return std::make_pair(boost::shared_ptr< Response >(), begin);
			else
			{ /* carry on */ }
			// the protocol and version is now in [where, whence)
			std::string protocol_and_version(where, whence);
			// there are only two protocol strings we support: HTTP/1.0 and HTTP/1.1
			if (protocol_and_version != "HTTP/1.0" && protocol_and_version != "HTTP/1.1")
				throw UnsupportedProtocol(protocol_and_version.begin(), protocol_and_version.end());
			else
			{ /* carry on */ }

			where = advanceThroughIgnorableWhiteSpace(whence, end);
			whence = advanceUntilIgnorableWhiteSpace(where, end);
			// if we're at the end of the buffer, we do not have the response line
			if (whence == end)
				return std::make_pair(boost::shared_ptr< Response >(), begin);
			else
			{ /* carry on */ }
			// the response code is now in [where, whence)
			std::string response_code(where, whence);
			where = advanceThroughIgnorableWhiteSpace(whence, end);
			whence = advanceUntilEndOfLine(where, end);
			// if we're at the end of the buffer, we do not have the entire status line
			if (whence == end)
				return std::make_pair(boost::shared_ptr< Response >(), begin);
			else
			{ /* carry on */ }
			std::string response_message(where, whence);

			return std::make_pair(boost::shared_ptr< Response >(new Response(protocol_and_version, boost::lexical_cast< unsigned int >(response_code), response_message)), whence);
		}
	}
}

#endif
