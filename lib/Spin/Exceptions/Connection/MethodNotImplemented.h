#ifndef _spin_exceptions_connection_methodnotimplemented_h
#define _spin_exceptions_connection_methodnotimplemented_h

#include <stdexcept>

namespace Spin
{
	namespace Exceptions
	{
		namespace Connection
		{
			struct MethodNotImplemented : std::logic_error
			{
				MethodNotImplemented() : std::logic_error("Method not implemented on this BIO")
				{ /* no-op */ }
			};
		}
	}
}

#endif
