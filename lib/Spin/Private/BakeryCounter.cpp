#include "BakeryCounter.h"
#if HAVE_ATOMIC_PRIMITIVES
#include "atomicPrimitives.h"
#endif

namespace Spin
{
	namespace Private
	{
		BakeryCounter::BakeryCounter()
		{
			counter_.u32_ = 0;
		}

		BakeryCounter::~BakeryCounter()
		{ /* no-op */ }

		void BakeryCounter::incrementClientCounter()
		{
#if HAVE_ATOMIC_PRIMITIVES
			BOOST_STATIC_ASSERT(client_counter__ == 0);
			atomicIncrement(counter_.u32_);
#else
			boost::mutex::scoped_lock lock(lock_);
			++(counter_.u16_[client_counter__]);
#endif
		}

		void BakeryCounter::incrementBakerCounter()
		{
#if HAVE_ATOMIC_PRIMITIVES
			BOOST_STATIC_ASSERT(baker_counter__ == 1);
			atomicAdd(counter_.u32_, 0x00010000);
#else
			boost::mutex::scoped_lock lock(lock_);
			++(counter_.u16_[baker_counter__]);
#endif
		}
	}
}
