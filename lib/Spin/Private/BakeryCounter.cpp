#include "BakeryCounter.h"
#include <ace/Atomic_Op.h>
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
			lock_.acquire();
			++(counter_.u16_[client_counter__]);
			lock_.release();
#endif
		}

		void BakeryCounter::incrementBakerCounter()
		{
#if HAVE_ATOMIC_PRIMITIVES
			BOOST_STATIC_ASSERT(baker_counter__ == 1);
			atomicAdd(counter_.u32_, 0x00010000);
#else
			lock_.acquire();
			++(counter_.u16_[baker_counter__]);
			lock_.release();
#endif
		}
	}
}
