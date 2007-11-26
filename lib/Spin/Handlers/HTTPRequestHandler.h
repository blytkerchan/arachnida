#ifndef _spin_handlers_httprequesthandler_h
#define _spin_handlers_httprequesthandler_h

#include <boost/shared_ptr.hpp>

namespace Spin
{
	namespace Details
	{
		struct Request;
	}
	namespace Handlers
	{
		class HTTPRequestHandler
		{
		public :
			void handle(const boost::shared_ptr< Details::Request > & request);
		};
	}
}

#endif
