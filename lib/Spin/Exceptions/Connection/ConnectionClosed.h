#ifndef _spin_exceptions_connection_connectionclosed_h
#define _spin_exceptions_connection_connectionclosed_h

#include <stdexcept>

namespace Spin
{
	namespace Exceptions
	{
		namespace Connection
		{
			struct ConnectionClosed : std::logic_error
			{
				ConnectionClosed() : std::logic_error("Connection closed")
				{ /* no-op */ }
			};
		}
	}
}

#endif
