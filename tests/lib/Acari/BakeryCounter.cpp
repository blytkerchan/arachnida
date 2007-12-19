#include "BakeryCounter.h"
#include <Acari/BakeryCounter.h>

namespace Tests
{
	namespace Acari
	{
		CPPUNIT_TEST_SUITE_REGISTRATION(BakeryCounter);

		void BakeryCounter::setUp()
		{ /* no-op */ }

		void BakeryCounter::tearDown()
		{ /* no-op */ }

		void BakeryCounter::tryIncrementClient()
		{
			::Acari::BakeryCounter counter;
			CPPUNIT_ASSERT(counter.getClientCounter() == 0);
			counter.incrementClientCounter();
			CPPUNIT_ASSERT(counter.getClientCounter() == 1);
		}

		void BakeryCounter::tryIncrementBaker()
		{
			::Acari::BakeryCounter counter;
			CPPUNIT_ASSERT(counter.getBakerCounter() == 0);
			counter.incrementBakerCounter();
			CPPUNIT_ASSERT(counter.getBakerCounter() == 1);
		}
	}
}
