#ifndef _spin_exceptions_connection_unusuableconnection_h
#define _spin_exceptions_connection_unusuableconnection_h

#include <stdexcept>

namespace Spin
{
	namespace Exceptions
	{
		namespace Connection
		{
			struct UnusableConnection : std::logic_error
			{
				UnusableConnection() : std::logic_error("Connection no longer usable")
				{ /* no-op */ }
			};
		}
	}
}

#endif
