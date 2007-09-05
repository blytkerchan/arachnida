#ifndef _tests_spin_private_countedevent_h
#define _tests_spin_private_countedevent_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Spin
	{
		namespace Private
		{
			class CountedEvent : public CPPUNIT_NS::TestFixture
			{
				CPPUNIT_TEST_SUITE( CountedEvent );
				CPPUNIT_TEST(tryFiveThreads);
				CPPUNIT_TEST_SUITE_END();

			public:
				virtual void setUp();
				virtual void tearDown();

			protected:
				void tryFiveThreads();
				void tryIncrementBaker();
			private:
			};
		}
	}
}

#endif
