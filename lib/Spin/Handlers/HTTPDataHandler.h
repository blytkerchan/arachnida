#ifndef _spin_handlers_httpdatahandler_h
#define _spin_handlers_httpdatahandler_h

#include "NewDataHandler.h"
#include <map>
#include <boost/thread/once.hpp>

namespace Spin
{
	class Connection;
	namespace Handlers
	{
		class HTTPRequestHandler;
		/** Data handler for HTTP connections.
		 * This data handler will parse whatever the connection has to offer in 
		  * terms of data as an HTTP request, try to extract the request from that 
		  * data and pass the request to the request handler. The request handler 
		  * in question is usually passed to it by the HTTPConnectionHandler. */
		class HTTPDataHandler : public NewDataHandler
		{
		public :
			//! Create a new data handler with a given request handler
			HTTPDataHandler(HTTPRequestHandler & request_handler);

		protected :
			/*virtual */void onDataReady(Connection & connection) const/* = 0*/;

		private :
			enum Method { options__, get__, head__, post__, put__, delete__, trace__, connect__ };

			HTTPRequestHandler & request_handler_;

			static void initStaticMembers();

			static boost::once_flag once_flag__;
			static unsigned long attribute_index__;
			static std::map< std::string, Method > supported_methods__;
		};
	}
}

#endif
