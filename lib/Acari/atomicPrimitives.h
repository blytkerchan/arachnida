#ifndef _acari_atomicprimitives_h
#define _acari_atomicprimitives_h

#include "Details/prologue.h"
#include <boost/cstdint.hpp>

namespace Acari
{
	ACARI_API void atomicIncrement(volatile boost::uint32_t & u32);
	ACARI_API void atomicAdd(volatile boost::uint32_t & u32, boost::uint32_t val);
	ACARI_API boost::uint32_t fetchAndIncrement(volatile boost::uint32_t & u32);
	ACARI_API boost::uint32_t fetchAndDecrement(volatile boost::uint32_t & u32);
	ACARI_API void atomicSet(volatile int & target, int value);
	ACARI_API void atomicSet(volatile void * & target, void * value);
}

#endif
