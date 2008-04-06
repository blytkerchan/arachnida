#ifndef _spin_exceptions_connection_bindfailure_h
#define _spin_exceptions_connection_bindfailure_h

#include <algorithm>

namespace Spin
{
	namespace Exceptions
	{
		namespace Connection
		{
			//! Thrown when the implementation failed to bind to a given port
			struct BindFailure : public ConnectionError
			{
				//! Construct a bind failure with an error code provided by the OS
				BindFailure(int error_code)
					: ConnectionError("Bind failure."),
					  error_code_(error_code),
					  what_(0)
				{ /* no-op */ }

				//! Copy-construct a bind failure
				BindFailure(const BindFailure & e)
					: ConnectionError(e),
					  error_code_(e.error_code_),
					  what_(e.what_)
				{
					e.what_ = 0;
				}

				//! No-fail destructor
				~BindFailure() throw()
				{
					delete[] what_; // Assuming what_ was created with new[]
				}

				//! Assignment
				BindFailure & operator=(BindFailure e)
				{ return swap(e); }

				//! No-fail swap
				BindFailure & swap(BindFailure & e) throw()
				{
					std::swap(what_, e.what_);
					std::swap(error_code_, e.error_code_);
					return *this;
				}

				//! No-fail what, will return a default string if anything does fail
				virtual const char * what() const throw();

				int error_code_;
				mutable char * what_;
			};
		}
	}
}

#endif
