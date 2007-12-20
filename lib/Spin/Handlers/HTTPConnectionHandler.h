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
		/** A handler for new HTTP connections.
		 * The default handler will accept any and all new HTTP connections.
		 * You can derive from this class and override the validate method to
		 * change this behavior and filter addresses from which to accept or
		 * not accept connections.
		 * 
		 * You can also override the handleNewConnection method and customize 
		 * this handler even more, but in that case you might as well derive 
		 * directly from NewConnectionHandler. */
		class SPIN_API HTTPConnectionHandler : public NewConnectionHandler
		{
		public :
			//! Construct a connection handler with a given request handler, which will be installed on all new connections.
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
