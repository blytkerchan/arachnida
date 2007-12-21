#ifndef _scorpion_exceptions_ssl_connectionsocketcreationfailure_h
#define _scorpion_exceptions_ssl_connectionsocketcreationfailure_h

namespace Scorpion
{
	namespace Exceptions
	{
		namespace SSL
		{
			//! Thrown when the connection socket could not be created
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
