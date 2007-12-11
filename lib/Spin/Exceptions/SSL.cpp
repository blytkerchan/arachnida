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
			return std::runtime_error::what();
		}
	}
}


