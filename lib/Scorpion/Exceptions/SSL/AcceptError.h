#ifndef _scorpion_exceptions_ssl_accepterror_h
#define _scorpion_exceptions_ssl_accepterror_h

namespace Scorpion
{
	namespace Exceptions
	{
		namespace SSL
		{
			//! Thrown when accept returns an error
			struct AcceptError : public SSLProtocolError
			{
				AcceptError(int error_code, const char * filename, int line, const char * error_line_data, int error_line_data_flags)
					: SSLProtocolError("Error accepting connection", error_code, filename, line, error_line_data, error_line_data_flags)
				{ /* no-op */ }
			};
		}
	}
}

#endif
