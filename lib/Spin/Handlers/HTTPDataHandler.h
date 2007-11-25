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
		class HTTPDataHandler : public NewDataHandler
		{
		public :
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
