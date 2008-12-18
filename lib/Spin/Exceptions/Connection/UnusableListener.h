#ifndef _spin_exceptions_connection_unusuablelistener_h
#define _spin_exceptions_connection_unusuablelistener_h

#include <stdexcept>

namespace Spin
{
	namespace Exceptions
	{
		namespace Connection
		{
			//! Thrown when a connection is used after it has declared itself unusable
			struct UnusableListener : std::logic_error
			{
				UnusableListener() : std::logic_error("Listener no longer usable")
				{ /* no-op */ }
			};
		}
	}
}

#endif
