#ifndef _spin_private_countedevent_h
#define _spin_private_countedevent_h

#include "../Details/prologue.h"
#include <ace/Event.h>
#if !HAVE_ATOMIC_PRIMITIVES
#	include <ace/Thread_Mutex.h>
#	include <ace/RW_Thread_Mutex.h>
#endif
#include <boost/cstdint.hpp>

namespace Spin
{
	namespace Private
	{
		/* This thing is only necessary because condition variables are so un-portable. */
		class SPIN_API CountedEvent
		{
		public :
			CountedEvent();
			~CountedEvent();

			void wait();
			void signal();
			void switch_();

		private :
			CountedEvent(const CountedEvent&);
			CountedEvent & operator=(const CountedEvent&);

			int current_;
			ACE_Event * events_[2];
			volatile boost::uint32_t wait_count_[2];
#if !HAVE_ATOMIC_PRIMITIVES
			ACE_Thread_Mutex lock_;
			ACE_RW_Thread_Mutex rwlock_;
#endif
		};
	}
}

#endif
