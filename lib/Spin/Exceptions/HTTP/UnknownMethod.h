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
			struct SPIN_API UnknownMethod : public HTTPProtocolError
			{
				enum {
					max_method_size__ = 96	///< Maximum length of a method
				};

				//! Construct from a range of chars containing the method name
				template < typename Iterator >
				UnknownMethod(Iterator begin, Iterator end)
					: HTTPProtocolError("Unknown method."),
					  what_(0)
				{
					Iterator where(end);
					if (std::distance(begin, where) > max_method_size__)
					{
						where = begin;
						std::advance(where, static_cast< typename std::iterator_traits< Iterator >::difference_type >(max_method_size__));
					}
					else
					{ /* all is well */ }
					char * start(std::copy(begin, where, protocol_));
					*start = 0;
				}

				//! Copy-construct the exception
				UnknownMethod(const UnknownMethod & e)
					: HTTPProtocolError(e),
					  what_(e.what_)
				{
					std::copy(e.protocol_, e.protocol_ + max_method_size__, protocol_);
					e.what_ = 0;
				}

				//! No-fail destructor
				~UnknownMethod() throw()
				{
					delete[] what_; // Assuming what_ was created with new[]
				}

				//! Assignment operator
				UnknownMethod & operator=(UnknownMethod e)
				{ return swap(e); }

				//! No-fail swap
				UnknownMethod & swap(UnknownMethod & e) throw()
				{
					std::swap_ranges(e.protocol_, e.protocol_ + max_method_size__, protocol_);
					std::swap(what_, e.what_);
					return *this;
				}

				//! No-fail what, will return a default string if anything does fail
				virtual const char * what() const throw();

				char protocol_[max_method_size__ + 1];
				mutable char * what_;
			};
		}
	}
}

#endif
