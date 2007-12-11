#ifndef _spin_exceptions_ssl_sslcontextallocationerror_h
#define _spin_exceptions_ssl_sslcontextallocationerror_h

namespace Spin
{
	namespace Exceptions
	{
		namespace SSL
		{
			struct SSLContextAllocationError : public SSLProtocolError
			{
				SSLContextAllocationError(int error_code, const char * filename, int line, const char * error_line_data, int error_line_data_flags)
					: SSLProtocolError("Error allocating SSL context", error_code, filename, line, error_line_data, error_line_data_flags)
				{ /* no-op */ }
			};
		}
	}
}

#endif
