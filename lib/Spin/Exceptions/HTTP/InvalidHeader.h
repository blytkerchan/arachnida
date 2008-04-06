#ifndef _spin_exceptions_http_invalidheader_h
#define _spin_exceptions_http_invalidheader_h

#include <algorithm>
#include <iterator>

namespace Spin
{
	namespace Exceptions
	{
		namespace HTTP
		{
			//! Thrown when an invalid header is found in a request
			struct SPIN_API InvalidHeader : public HTTPProtocolError
			{
				enum {
					max_header_size__ = 96	///< the maximum size of the header, as supported by this exception. If you need more than this, increase and re-compile the library and \em all client code
				};

				//! Construct from a range containing the header (assumed to be a range of chars)
				template < typename Iterator >
				InvalidHeader(Iterator begin, Iterator end)
					: HTTPProtocolError("Invalid header field."),
					  what_(0)
				{
					Iterator where(end);
					if (std::distance(begin, where) > max_header_size__)
					{
						where = begin;
						std::advance(where, static_cast< typename std::iterator_traits< Iterator >::difference_type >(max_header_size__));
					}
					else
					{ /* all is well */ }
					char * start(std::copy(begin, where, header_content_));
					*start = 0;
				}

				//! Copy-construct the exception
				InvalidHeader(const InvalidHeader & e)
					: HTTPProtocolError(e),
					  what_(e.what_)
				{
					std::copy(e.header_content_, e.header_content_ + max_header_size__, header_content_);
					e.what_ = 0;
				}

				//! No-fail destructor
				~InvalidHeader() throw()
				{
					delete[] what_; // Assuming what_ was created with new[]
				}

				//! Assignment operator
				InvalidHeader & operator=(InvalidHeader e)
				{ return swap(e); }

				//! No-fail swap
				InvalidHeader & swap(InvalidHeader & e) throw()
				{
					std::swap_ranges(e.header_content_, e.header_content_ + max_header_size__, header_content_);
					std::swap(what_, e.what_);
					return *this;
				}

				//! No-fail what - returns a default string if anything does fail
				virtual const char * what() const throw();

				char header_content_[max_header_size__ + 1];
				mutable char * what_;
			};
		}
	}
}

#endif
