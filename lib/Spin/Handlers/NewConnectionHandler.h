#ifndef _spin_handlers_newconnectionhandler_h
#define _spin_handlers_newconnectionhandler_h

#include "../Details/prologue.h"
#include <memory>

class ACE_SOCK_Stream;
namespace Spin
{
	namespace Handlers
	{
		class SPIN_API NewConnectionHandler
		{
		public :
			virtual ~NewConnectionHandler();

			NewConnectionHandler & operator()(std::auto_ptr< ACE_SOCK_Stream > & new_connection)
			{
				handleNewConnection(new_connection);
				return *this;
			}

		protected :
			virtual void handleNewConnection(std::auto_ptr< ACE_SOCK_Stream > & new_connection) = 0;
		};
	}
}

#endif
