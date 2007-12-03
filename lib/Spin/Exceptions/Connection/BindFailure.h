#ifndef _spin_exceptions_connection_bindfailure_h
#define _spin_exceptions_connection_bindfailure_h

#include <algorithm>

namespace Spin
{
	namespace Exceptions
	{
		namespace Connection
		{
			struct BindFailure : public ConnectionError
			{
				BindFailure(int error_code)
					: ConnectionError("Bind failure."),
					  error_code_(error_code)
				{ /* no-op */ }

				BindFailure(const BindFailure & e)
					: ConnectionError(e),
					  error_code_(e.error_code_)
				{ /* no-op */ }

				~BindFailure()
				{
					delete[] what_; // Assuming what_ was created with new[]
				}

				BindFailure & operator=(BindFailure e)
				{ return swap(e); }

				BindFailure & swap(BindFailure & e) throw()
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
