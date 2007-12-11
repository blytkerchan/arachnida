#ifndef _spin_exceptions_ssl_connectionerror_h
#define _spin_exceptions_ssl_connectionerror_h

namespace Spin
{
	namespace Exceptions
	{
		namespace SSL
		{
			struct ConnectionError : public SSLProtocolError
			{
				ConnectionError(int error_code, const char * filename, int line, const char * error_line_data, int error_line_data_flags)
					: SSLProtocolError("Connection error", error_code, filename, line, error_line_data, error_line_data_flags)
				{ /* no-op */ }
			};
		}
	}
}

#endif
