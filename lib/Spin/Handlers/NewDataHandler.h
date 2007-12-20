#ifndef _spin_handlers_newdatahandler_h
#define _spin_handlers_newdatahandler_h

#include "../Details/prologue.h"

namespace Spin
{
	class Connection;
	namespace Handlers
	{
		//! Handler for when new data is ready on a connection
		class SPIN_API NewDataHandler
		{
		public :
			const NewDataHandler & operator()(Connection & connection) const
			{
				onDataReady(connection);
				return *this;
			}

		protected :
			virtual ~NewDataHandler();
			//! Called when new data is ready.
			virtual void onDataReady(Connection & connection) const = 0;
		};
	}
}

#endif
