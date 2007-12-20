#ifndef _spin_exceptions_connection_unusuableconnection_h
#define _spin_exceptions_connection_unusuableconnection_h

#include <stdexcept>

namespace Spin
{
	namespace Exceptions
	{
		namespace Connection
		{
			//! Thrown when a connection is used after it has declared itself unusable
			struct UnusableConnection : std::logic_error
			{
				UnusableConnection() : std::logic_error("Connection no longer usable")
				{ /* no-op */ }
			};
		}
	}
}

#endif
