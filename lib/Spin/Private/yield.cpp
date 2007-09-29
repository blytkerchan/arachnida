#include "yield.h"
#if defined(_WIN32) || defined(__CYGWIN__)
#include <Windows.h>
#else
extern "C" {
#include <unistd.h>
#if defined(_POSIX_PRIORITY_SCHEDULING)
#include <sched.h>
#else
#error "sched_yield not available"
#endif
}
#endif

namespace Spin
{
	namespace Private
	{
		void yield(bool to_lower/* = false*/)
		{
#if defined(_WIN32) || defined(__CYGWIN__)
			::Sleep(to_lower ? 1 : 0);
#else
			::sched_yield();
#endif
		}
	}
}

