#ifndef _spin_exceptions_connection_methodnotimplemented_h
#define _spin_exceptions_connection_methodnotimplemented_h

#include <stdexcept>

namespace Spin
{
	namespace Exceptions
	{
		namespace Connection
		{
			//! Thrown when a method is used on a connection that does not implement it (should never happen)
			struct MethodNotImplemented : std::logic_error
			{
				MethodNotImplemented() : std::logic_error("Method not implemented on this BIO")
				{ /* no-op */ }
			};
		}
	}
}

#endif
