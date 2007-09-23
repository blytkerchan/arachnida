#include "atomicPrimitives.h"
#include <Spin/Private/atomicPrimitives.h>

namespace Tests
{
	namespace Spin
	{
		namespace Private
		{
			CPPUNIT_TEST_SUITE_REGISTRATION(atomicPrimitives);

			void atomicPrimitives::setUp()
			{ /* no-op */ }

			void atomicPrimitives::tearDown()
			{ /* no-op */ }

			void atomicPrimitives::tryIncrement()
			{
				volatile boost::uint32_t u32(0);
				::Spin::Private::atomicIncrement(u32);
				CPPUNIT_ASSERT(u32 == 1);
			}

			void atomicPrimitives::tryAdd()
			{
				volatile boost::uint32_t u32(0);
				::Spin::Private::atomicAdd(u32, 8732);
				CPPUNIT_ASSERT(u32 == 8732);
			}

			void atomicPrimitives::tryFetchAndDecrement()
			{
				volatile boost::uint32_t u32(0);
				boost::uint32_t val(::Spin::Private::fetchAndDecrement(u32));
				CPPUNIT_ASSERT(val == 0);
				CPPUNIT_ASSERT(u32 == boost::uint32_t(-1));
			}

			void atomicPrimitives::tryAtomicSet()
			{
				volatile int target(0);
				::Spin::Private::atomicSet(target, 12);
				CPPUNIT_ASSERT(target == 12);
			}
		}
	}
}
