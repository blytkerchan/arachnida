#ifndef _spin_handlers_httpconnectionhandler_h
#define _spin_handlers_httpconnectionhandler_h

#include "NewConnectionHandler.h"

namespace Spin
{
	namespace Details
	{
		struct Address;
	}
	namespace Handlers
	{
		class HTTPRequestHandler;
		class SPIN_API HTTPConnectionHandler : public NewConnectionHandler
		{
		public :
			HTTPConnectionHandler(HTTPRequestHandler & request_handler);

			~HTTPConnectionHandler();

		protected :
			/*virtual */void handleNewConnection(const Connection & connection)/* = 0*/;
			// overload this function and have it return false for addresses you don't want to accept connections from
			virtual bool validate(const Details::Address & peer_address);

		private :
			struct Data;

			Data * data_;
			HTTPRequestHandler & request_handler_;
		};
	}
}

#endif
