#ifndef _spin_handlers_newconnectionhandler_h
#define _spin_handlers_newconnectionhandler_h

#include "../Details/prologue.h"
#include <boost/shared_ptr.hpp>

namespace Spin
{
	class Connection;
	namespace Handlers
	{
		//! Base class for handlers of new connections.
		class SPIN_API NewConnectionHandler
		{
		public :
			virtual ~NewConnectionHandler();

			NewConnectionHandler & operator()(boost::shared_ptr< Connection > connection)
			{
				handleNewConnection(connection);
				return *this;
			}

		protected :
			//! Called whenever a new connection is ready.
			virtual void handleNewConnection(boost::shared_ptr< Connection > connection) = 0;
		};
	}
}

#endif
