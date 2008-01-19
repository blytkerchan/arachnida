#ifndef _spin_exceptions_http_h
#define _spin_exceptions_http_h

#include "../Details/prologue.h"
#include <stdexcept>

namespace Spin
{
	namespace Exceptions
	{
		//! Base class for all HTTP protocol related errors
		struct SPIN_API HTTPProtocolError : std::runtime_error
		{
			HTTPProtocolError(const char * what)
				: std::runtime_error(what)
			{ /* no-op */ }

			HTTPProtocolError(const HTTPProtocolError &e)
				: std::runtime_error(e)
			{ /* no-op */ }
		};
	}
}

#include "HTTP/InvalidHeader.h"
#include "HTTP/UnsupportedProtocol.h"
#include "HTTP/UnknownMethod.h"

#endif

