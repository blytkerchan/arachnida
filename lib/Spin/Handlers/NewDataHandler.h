#ifndef _spin_handlers_newdatahandler_h
#define _spin_handlers_newdatahandler_h

#include "../Details/prologue.h"

namespace Spin
{
	namespace Handlers
	{
		class SPIN_API NewDataHandler
		{
		public :
			const NewDataHandler & operator()() const
			{
				onDataReady();
				return *this;
			}

		protected :
			~NewDataHandler();
			virtual void onDataReady() const throw() = 0;
		};
	}
}

#endif
