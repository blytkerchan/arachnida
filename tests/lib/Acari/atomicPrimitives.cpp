#include "atomicPrimitives.h"
#include <Acari/atomicPrimitives.h>

namespace Tests
{
	namespace Acari
	{
		CPPUNIT_TEST_SUITE_REGISTRATION(atomicPrimitives);

		void atomicPrimitives::setUp()
		{ /* no-op */ }

		void atomicPrimitives::tearDown()
		{ /* no-op */ }

		void atomicPrimitives::tryIncrement()
		{
			volatile boost::uint32_t u32(0);
			::Acari::atomicIncrement(u32);
			CPPUNIT_ASSERT(u32 == 1);
		}

		void atomicPrimitives::tryAdd()
		{
			volatile boost::uint32_t u32(0);
			::Acari::atomicAdd(u32, 8732);
			CPPUNIT_ASSERT(u32 == 8732);
		}

		void atomicPrimitives::tryFetchAndIncrement()
		{
			volatile boost::uint32_t u32(0);
			boost::uint32_t val(::Acari::fetchAndIncrement(u32));
			CPPUNIT_ASSERT(val == 0);
			CPPUNIT_ASSERT(u32 == boost::uint32_t(1));
		}

		void atomicPrimitives::tryFetchAndDecrement()
		{
			volatile boost::uint32_t u32(0);
			boost::uint32_t val(::Acari::fetchAndDecrement(u32));
			CPPUNIT_ASSERT(val == 0);
			CPPUNIT_ASSERT(u32 == boost::uint32_t(-1));
		}

		void atomicPrimitives::tryAtomicSet()
		{
			volatile int target(0);
			::Acari::atomicSet(target, 12);
			CPPUNIT_ASSERT(target == 12);
		}
	}
}
