#ifndef _acari_atomicprimitives_h
#define _acari_atomicprimitives_h

#include "Details/prologue.h"
#include <boost/cstdint.hpp>

namespace Acari
{
	//! Atomically increment the given integer
	ACARI_API void atomicIncrement(volatile boost::uint32_t & u32);
	//! Atomically add the given value to the given integer
	ACARI_API void atomicAdd(volatile boost::uint32_t & u32, boost::uint32_t val);
	//! Atomically post-increment the given integer
	ACARI_API boost::uint32_t fetchAndIncrement(volatile boost::uint32_t & u32);
	//! Atomically post-decrement the given integer
	ACARI_API boost::uint32_t fetchAndDecrement(volatile boost::uint32_t & u32);
	//! Atomically set the given integer to the given value
	ACARI_API void atomicSet(volatile int & target, int value);
	//! Atomically set the given pointer to the given value
	ACARI_API void atomicSet(volatile void * & target, void * value);
}

#endif
