#ifndef _spin_exceptions_connection_unusuableudpsocket_h
#define _spin_exceptions_connection_unusuableudpsocket_h

#include <stdexcept>

namespace Spin
{
	namespace Exceptions
	{
		namespace Connection
		{
			//! Thrown when a UDP socket is used after it has declared itself unusable
			struct UnusableUDPSocket : std::logic_error
			{
				UnusableUDPSocket() : std::logic_error("UDPSocket no longer usable")
				{ /* no-op */ }
			};
		}
	}
}

#endif
