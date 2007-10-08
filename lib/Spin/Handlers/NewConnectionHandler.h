#ifndef _spin_handlers_newconnectionhandler_h
#define _spin_handlers_newconnectionhandler_h

#include "../Details/prologue.h"
#include <memory>

class ACE_SOCK_Stream;
namespace Spin
{
	class Connection;
	namespace Handlers
	{
		class SPIN_API NewConnectionHandler
		{
		public :
			virtual ~NewConnectionHandler();

			NewConnectionHandler & operator()(const Connection & connection)
			{
				handleNewConnection(connection);
				return *this;
			}

		protected :
			virtual void handleNewConnection(const Connection & connection) = 0;
		};
	}
}

#endif
