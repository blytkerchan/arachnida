#ifndef _scorpion_exceptions_ssl_serversocketallocationerror_h
#define _scorpion_exceptions_ssl_serversocketallocationerror_h

namespace Scorpion
{
	namespace Exceptions
	{
		namespace SSL
		{
			//! Thrown when the implementation fails to allocate a server socket
			struct ServerSocketAllocationError : public SSLProtocolError
			{
				ServerSocketAllocationError(int error_code, const char * filename, int line, const char * error_line_data, int error_line_data_flags)
					: SSLProtocolError("Error allocating server socket", error_code, filename, line, error_line_data, error_line_data_flags)
				{ /* no-op */ }
			};
		}
	}
}

#endif
