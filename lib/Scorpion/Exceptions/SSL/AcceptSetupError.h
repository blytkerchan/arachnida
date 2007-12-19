#ifndef _spin_exceptions_ssl_acceptsetuperror_h
#define _spin_exceptions_ssl_acceptsetuperror_h

namespace Scorpion
{
	namespace Exceptions
	{
		namespace SSL
		{
			struct AcceptSetupError : public SSLProtocolError
			{
				AcceptSetupError(int error_code, const char * filename, int line, const char * error_line_data, int error_line_data_flags)
					: SSLProtocolError("Error setting up accept socket", error_code, filename, line, error_line_data, error_line_data_flags)
				{ /* no-op */ }
			};
		}
	}
}

#endif
