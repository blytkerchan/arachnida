#ifndef _spin_exceptions_http_invalidheader_h
#define _spin_exceptions_http_invalidheader_h

#include <algorithm>

namespace Spin
{
	namespace Exceptions
	{
		namespace HTTP
		{
			struct InvalidHeader : public HTTPProtocolError
			{
				enum { max_header_size__ = 96 };

				template < typename Iterator >
				InvalidHeader(Iterator begin, Iterator end)
					: HTTPProtocolError("Invalid header field.")
				{
					Iterator where(end);
					if (std::distance(begin, where) > max_header_size__)
					{
						where = begin;
						std::advance(where, max_header_size__);
					}
					else
					{ /* all is well */ }
					char * start(std::copy(begin, where, header_content_));
					*start = 0;
				}

				InvalidHeader(const InvalidHeader & e)
					: HTTPProtocolError(e)
				{
					std::copy(e.header_content_, e.header_content_ + max_header_size__, header_content_);
					e.what_ = 0;
				}

				~InvalidHeader()
				{
					delete[] what_; // Assuming what_ was created with new[]
				}

				InvalidHeader & operator=(InvalidHeader e)
				{ return swap(e); }

				InvalidHeader & swap(InvalidHeader & e) throw()
				{
					std::swap_ranges(e.header_content_, e.header_content_ + max_header_size__, header_content_);
					std::swap(what_, e.what_);
					return *this;
				}

				virtual const char * what() const throw();

				char header_content_[max_header_size__ + 1];
				mutable char * what_;
			};
		}
	}
}

#endif
