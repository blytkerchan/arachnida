#ifndef _spin_handlers_udpdatahandler_h
#define _spin_handlers_udpdatahandler_h

#include "../Details/prologue.h"
#include <boost/shared_ptr.hpp>

namespace Spin
{
	class UDPSocket;
	namespace Handlers
	{
		//! Handler for when new data is ready on a UDP socket
		class SPIN_API UDPDataHandler
		{
		public :
			const UDPDataHandler & operator()(boost::shared_ptr< UDPSocket > socket) const
			{
				onDataReady(socket.get());
				return *this;
			}

			const UDPDataHandler & operator()(UDPSocket * socket) const
			{
				onDataReady(socket);
				return *this;
			}

		protected :
			virtual ~UDPDataHandler();
			//! Called when new data is ready.
			virtual void onDataReady(UDPSocket * socket) const = 0;
		};
	}
}

#endif
