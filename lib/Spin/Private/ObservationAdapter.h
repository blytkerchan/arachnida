#ifndef _spin_private_observationadapter_h
#define _spin_private_observationadapter_h

#include <ace/ACE.h>

namespace Spin
{
	namespace Private
	{
		struct ObservationAdapter
		{
		public :
			virtual ACE_HANDLE getHandle() const = 0;
			virtual void onReadReady();
			virtual void onWriteReady();
			virtual void onExceptionReady();

		protected :
			virtual ~ObservationAdapter();
		};
	}
}

#endif