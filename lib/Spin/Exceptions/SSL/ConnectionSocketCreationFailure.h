#ifndef _spin_exceptions_ssl_connectionsocketcreationfailure_h
#define _spin_exceptions_ssl_connectionsocketcreationfailure_h

namespace Spin
{
	namespace Exceptions
	{
		namespace SSL
		{
			struct ConnectionSocketCreationFailure : public SSLProtocolError
			{
				ConnectionSocketCreationFailure(int error_code, const char * filename, int line, const char * error_line_data, int error_line_data_flags)
					: SSLProtocolError("Failed to create connection socket", error_code, filename, line, error_line_data, error_line_data_flags)
				{ /* no-op */ }
			};
		}
	}
}

#endif
