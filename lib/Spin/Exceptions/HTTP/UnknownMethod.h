#ifndef _spin_exceptions_http_unknownmethod_h
#define _spin_exceptions_http_unknownmethod_h

#include <algorithm>
#include <iterator>

namespace Spin
{
	namespace Exceptions
	{
		namespace HTTP
		{
			//! Thrown when an unknown request method is used by the client
			struct UnknownMethod : public HTTPProtocolError
			{
				enum { max_protocol_size__ = 96 };

				template < typename Iterator >
				UnknownMethod(Iterator begin, Iterator end)
					: HTTPProtocolError("Unknown method.")
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

				UnknownMethod(const UnknownMethod & e)
					: HTTPProtocolError(e)
				{
					std::copy(e.protocol_, e.protocol_ + max_protocol_size__, protocol_);
					e.what_ = 0;
				}

				~UnknownMethod() throw()
				{
					delete[] what_; // Assuming what_ was created with new[]
				}

				UnknownMethod & operator=(UnknownMethod e)
				{ return swap(e); }

				UnknownMethod & swap(UnknownMethod & e) throw()
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
