#ifndef _spin_exceptions_http_unsupportedprotocol_h
#define _spin_exceptions_http_unsupportedprotocol_h

#include <algorithm>
#include <iterator>

namespace Spin
{
	namespace Exceptions
	{
		namespace HTTP
		{
			//! Thrown when an unsupported protocol and/or protocol version is used
			struct UnsupportedProtocol : public HTTPProtocolError
			{
				enum { max_protocol_size__ = 96 };

				template < typename Iterator >
				UnsupportedProtocol(Iterator begin, Iterator end)
					: HTTPProtocolError("Unsupported protocol.")
				{
					Iterator where(end);
					if (std::distance(begin, where) > max_protocol_size__)
					{
						where = begin;
						std::advance(where, static_cast< typename std::iterator_traits< Iterator >::difference_type >(max_protocol_size__));
					}
					else
					{ /* all is well */ }
					char * start(std::copy(begin, where, protocol_));
					*start = 0;
				}

				UnsupportedProtocol(const UnsupportedProtocol & e)
					: HTTPProtocolError(e)
				{
					std::copy(e.protocol_, e.protocol_ + max_protocol_size__, protocol_);
					e.what_ = 0;
				}

				~UnsupportedProtocol() throw()
				{
					delete[] what_; // Assuming what_ was created with new[]
				}

				UnsupportedProtocol & operator=(UnsupportedProtocol e)
				{ return swap(e); }

				UnsupportedProtocol & swap(UnsupportedProtocol & e) throw()
				{
					std::swap_ranges(e.protocol_, e.protocol_ + max_protocol_size__, protocol_);
					std::swap(what_, e.what_);
					return *this;
				}

				virtual const char * what() const throw();

				char protocol_[max_protocol_size__ + 1];
				mutable char * what_;
			};
		}
	}
}

#endif
