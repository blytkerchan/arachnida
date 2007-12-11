#include "SSL.h"
extern "C"
{
#include <openssl/err.h>
}

namespace Spin
{
	namespace Exceptions
	{
		/*virtual */const char * SSLProtocolError::what() throw()
		{
			const char * filename(0);
			int line(0);
			::ERR_get_error_line(&filename, &line);
		}
	}
}


