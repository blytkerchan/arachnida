#ifndef _acari_scopedmutexlock_h
#define _acari_scopedmutexlock_h

namespace Acari
{
	class Mutex;
	class ScopedMutexLock
	{
	public :
		ScopedMutexLock(Mutex &mutex);
		~ScopedMutexLock();

	private :
		ScopedMutexLock(const ScopedMutexLock&);
		ScopedMutexLock& operator=(const ScopedMutexLock&);

		Mutex &mutex_;
	};
}

#endif
