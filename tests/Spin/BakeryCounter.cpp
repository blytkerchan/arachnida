#include "BakeryCounter.h"
#include <Spin/Private/BakeryCounter.h>

namespace Tests
{
	namespace Spin
	{
		namespace Private
		{
			CPPUNIT_TEST_SUITE_REGISTRATION(BakeryCounter);

			void BakeryCounter::setUp()
			{ /* no-op */ }

			void BakeryCounter::tearDown()
			{ /* no-op */ }

			void BakeryCounter::tryIncrementClient()
			{
				::Spin::Private::BakeryCounter counter;
				CPPUNIT_ASSERT(counter.getClientCounter() == 0);
				counter.incrementClientCounter();
				CPPUNIT_ASSERT(counter.getClientCounter() == 1);
			}

			void BakeryCounter::tryIncrementBaker()
			{
				::Spin::Private::BakeryCounter counter;
				CPPUNIT_ASSERT(counter.getBakerCounter() == 0);
				counter.incrementBakerCounter();
				CPPUNIT_ASSERT(counter.getBakerCounter() == 1);
			}
		}
	}
}
