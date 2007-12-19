#include "BakeryCounter.h"
#include "atomicPrimitives.h"

namespace Acari
{
	BakeryCounter::BakeryCounter()
	{
		counter_.u32_ = 0;
	}

	BakeryCounter::~BakeryCounter()
	{ /* no-op */ }

	void BakeryCounter::incrementClientCounter()
	{
		BOOST_STATIC_ASSERT(client_counter__ == 0);
		atomicIncrement(counter_.u32_);
	}

	void BakeryCounter::incrementBakerCounter()
	{
		BOOST_STATIC_ASSERT(baker_counter__ == 1);
		atomicAdd(counter_.u32_, 0x00010000);
	}
}
