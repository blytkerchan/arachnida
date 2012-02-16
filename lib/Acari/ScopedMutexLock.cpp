#include "ScopedMutexLock.h"
#include "Mutex.h"

namespace Acari
{
	ScopedMutexLock::ScopedMutexLock(Mutex &mutex)
		: mutex_(mutex)
	{
		EnterCriticalSection(&mutex_.cs_);
	}

	ScopedMutexLock::~ScopedMutexLock()
	{
		LeaveCriticalSection(&mutex_.cs_);
	}
}


