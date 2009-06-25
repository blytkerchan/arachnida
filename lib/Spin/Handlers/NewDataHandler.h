#ifndef _spin_handlers_newdatahandler_h
#define _spin_handlers_newdatahandler_h

#include "../Details/prologue.h"
#include <boost/shared_ptr.hpp>

namespace Spin
{
	class Connection;
	namespace Handlers
	{
		//! Handler for when new data is ready on a connection
		class SPIN_API NewDataHandler
		{
		public :
			const NewDataHandler & operator()(boost::shared_ptr< Connection > connection)
			{
				onDataReady(connection);
				return *this;
			}

		protected :
			virtual ~NewDataHandler();
			//! Called when new data is ready.
			virtual void onDataReady(boost::shared_ptr< Connection > connection) = 0;
		};
	}
}

#endif
