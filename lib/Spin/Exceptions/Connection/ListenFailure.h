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
				//! Construct a listen failure with an error code provided by the OS
				ListenFailure(int error_code)
					: ConnectionError("Listen failure."),
					error_code_(error_code)
				{ /* no-op */ }

				//! Copy-construct a listen failure
				ListenFailure(const ListenFailure & e)
					: ConnectionError(e),
					error_code_(e.error_code_)
				{ /* no-op */ }

				//! No-fail destructor
				~ListenFailure() throw()
				{
					delete[] what_; // Assuming what_ was created with new[]
				}

				//! Assignment
				ListenFailure & operator=(ListenFailure e)
				{ return swap(e); }

				//! No-fail swap
				ListenFailure & swap(ListenFailure & e) throw()
				{
					std::swap(what_, e.what_);
					std::swap(error_code_, e.error_code_);
					return *this;
				}

				//! No-fail what, will return a default string if anything does fail
				virtual const char * what() const throw();

				int error_code_;		///< The error code from the OS
				mutable char * what_;
			};
		}
	}
}

#endif
