#ifndef _scorpion_exceptions_ssl_sslpointerlocationerror_h
#define _scorpion_exceptions_ssl_sslpointerlocationerror_h

namespace Scorpion
{
	namespace Exceptions
	{
		namespace SSL
		{
			//! Throw when the implementation fails to obtain the SSL pointer from a BIO
			struct SSLPointerLocationError : public SSLProtocolError
			{
				SSLPointerLocationError(int error_code, const char * filename, int line, const char * error_line_data, int error_line_data_flags)
					: SSLProtocolError("Failed to locate SSL pointer", error_code, filename, line, error_line_data, error_line_data_flags)
				{ /* no-op */ }
			};
		}
	}
}

#endif
