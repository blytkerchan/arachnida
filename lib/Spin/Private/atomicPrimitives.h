#ifndef _spin_private_atomicprimitives_h
#define _spin_private_atomicprimitives_h

#include "../Details/prologue.h"
#include <boost/cstdint.hpp>

namespace Spin
{
	namespace Private
	{
		SPIN_API void atomicIncrement(volatile boost::uint32_t & u32);
		SPIN_API void atomicAdd(volatile boost::uint32_t & u32, boost::uint32_t val);
		SPIN_API boost::uint32_t fetchAndIncrement(volatile boost::uint32_t & u32);
		SPIN_API boost::uint32_t fetchAndDecrement(volatile boost::uint32_t & u32);
		SPIN_API void atomicSet(volatile int & target, int value);
		SPIN_API void atomicSet(volatile void * & target, void * value);
	}
}

#endif
