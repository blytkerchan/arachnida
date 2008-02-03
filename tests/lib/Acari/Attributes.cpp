#include "Attributes.h"
#include <Acari/Attributes.h>

namespace Tests
{
	namespace Acari
	{
		CPPUNIT_TEST_SUITE_REGISTRATION(Attributes);

		void Attributes::setUp()
		{ /* no-op */ }

		void Attributes::tearDown()
		{ /* no-op */ }

		void Attributes::tryAllocateOne()
		{
			::Acari::Attributes attributes;
			CPPUNIT_ASSERT(attributes.allocateAttribute() == 0);
		}

		void Attributes::tryAllocateMany()
		{
			::Acari::Attributes attributes;
			unsigned int expected(1);
			while (expected < ::Acari::Attributes::max_attribute_count__)
			{
				CPPUNIT_ASSERT(attributes.allocateAttribute() == expected);
				++expected;
			}
			CPPUNIT_ASSERT_THROW(attributes.allocateAttribute(), std::bad_alloc);
		}
	}
}
