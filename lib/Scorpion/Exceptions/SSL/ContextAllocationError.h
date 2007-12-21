#ifndef _scorpion_exceptions_ssl_contextallocationerror_h
#define _scorpion_exceptions_ssl_contextallocationerror_h

namespace Scorpion
{
	namespace Exceptions
	{
		namespace SSL
		{
			//! Thrown when the SSL context could not be allocated
			struct ContextAllocationError : public SSLProtocolError
			{
				ContextAllocationError(int error_code, const char * filename, int line, const char * error_line_data, int error_line_data_flags)
					: SSLProtocolError("Context allocation failure", error_code, filename, line, error_line_data, error_line_data_flags)
				{ /* no-op */ }
			};
		}
	}
}

#endif
