#ifndef _spin_handlers_newdatahandler_h
#define _spin_handlers_newdatahandler_h

#include "../Details/prologue.h"

namespace Spin
{
	class Connection;
	namespace Handlers
	{
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
			virtual void onDataReady(Connection & connection) const throw() = 0;
		};
	}
}

#endif
