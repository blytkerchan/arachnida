#include "Mutex.h"

namespace Acari
{
	Mutex::Mutex()
	{
		InitializeCriticalSection(&cs_);
	}
	Mutex::~Mutex()
	{
		DeleteCriticalSection(&cs_);
	}
}


