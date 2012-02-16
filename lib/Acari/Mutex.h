#ifndef _acari_mutex_h
#define _acari_mutex_h

#include <windows.h>
#include "ScopedMutexLock.h"

namespace Acari
{
	class Mutex
	{
	public :
		typedef ScopedMutexLock scoped_lock;

		Mutex();
		~Mutex();

	private :
		Mutex(const Mutex&);
		Mutex& operator=(const Mutex&);

		CRITICAL_SECTION cs_;

		friend class ScopedMutexLock;
	};
}

#endif

