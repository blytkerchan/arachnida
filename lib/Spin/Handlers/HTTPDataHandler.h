#ifndef _spin_handlers_httpdatahandler_h
#define _spin_handlers_httpdatahandler_h

#include "NewDataHandler.h"

namespace Spin
{
	class Connection;
	namespace Handlers
	{
		class HTTPRequestHandler;
		class HTTPDataHandler : public NewDataHandler
		{
		public :
			HTTPDataHandler(HTTPRequestHandler & request_handler)
				: request_handler_(request_handler)
			{ /* no-op */ }

		protected :
			/*virtual */void onDataReady() const throw()/* = 0*/;

		private :
			HTTPRequestHandler & request_handler_;
		};
	}
}

#endif
