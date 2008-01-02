#ifndef _spin_exceptions_connection_listenfailure_h
#define _spin_exceptions_connection_listenfailure_h

#include <algorithm>

namespace Spin
{
	namespace Exceptions
	{
		namespace Connection
		{
			//! Thrown when the implementation can't listen on a given socket
			struct ListenFailure : public ConnectionError
			{
				ListenFailure(int error_code)
					: ConnectionError("Listen failure."),
					error_code_(error_code)
				{ /* no-op */ }

				ListenFailure(const ListenFailure & e)
					: ConnectionError(e),
					error_code_(e.error_code_)
				{ /* no-op */ }

				~ListenFailure() throw()
				{
					delete[] what_; // Assuming what_ was created with new[]
				}

				ListenFailure & operator=(ListenFailure e)
				{ return swap(e); }

				ListenFailure & swap(ListenFailure & e) throw()
				{
					std::swap(what_, e.what_);
					std::swap(error_code_, e.error_code_);
					return *this;
				}

				virtual const char * what() const throw();

				int error_code_;
				mutable char * what_;
			};
		}
	}
}

#endif
