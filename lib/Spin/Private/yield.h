#ifndef _spin_private_yield_h
#define _spin_private_yield_h

#include "../Details/prologue.h"

namespace Spin
{
	namespace Private
	{
		SPIN_API void yield(bool to_lower = false);
	}
}

#endif
