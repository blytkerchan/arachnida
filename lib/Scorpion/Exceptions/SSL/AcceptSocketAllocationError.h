#ifndef _scorpion_exceptions_ssl_acceptsocketallocationerror_h
#define _scorpion_exceptions_ssl_acceptsocketallocationerror_h

namespace Scorpion
{
	namespace Exceptions
	{
		namespace SSL
		{
			struct AcceptSocketAllocationError : public SSLProtocolError
			{
				AcceptSocketAllocationError(int error_code, const char * filename, int line, const char * error_line_data, int error_line_data_flags)
					: SSLProtocolError("Error allocating accept socket", error_code, filename, line, error_line_data, error_line_data_flags)
				{ /* no-op */ }
			};
		}
	}
}

#endif
