#ifndef _spin_exceptions_connection_h
#define _spin_exceptions_connection_h

#include <stdexcept>

namespace Spin
{
	namespace Exceptions
	{
		//! Base class for various errors pertaining connections and sockets
		struct ConnectionError : std::runtime_error
		{
			ConnectionError(const char * what)
				: std::runtime_error(what)
			{ /* no-op */ }

			ConnectionError(const ConnectionError &e)
				: std::runtime_error(e)
			{ /* no-op */ }
		};
	}
}

#include "Connection/BindFailure.h"
#include "Connection/ListenFailure.h"
#include "Connection/UnusableConnection.h"
#include "Connection/MethodNotImplemented.h"
#include "Connection/ConnectionClosed.h"

#endif

