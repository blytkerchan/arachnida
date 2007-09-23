#ifndef _spin_private_scopedlock_h
#define _spin_private_scopedlock_h

#include <ace/Thread_Mutex.h>

namespace Spin
{
	namespace Private
	{
		struct ScopedLock
		{
			ScopedLock(ACE_Thread_Mutex & lock)
				: lock_(lock)
			{
				lock_.acquire();
			}
			~ScopedLock()
			{
				lock_.release();
			}

			ACE_Thread_Mutex & lock_;

		private :
			// Neither CopyConstructible nor Assignable
			ScopedLock(const ScopedLock &);
			ScopedLock & operator=(const ScopedLock &);
		};
	}
}

#endif
