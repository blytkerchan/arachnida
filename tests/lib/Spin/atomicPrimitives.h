#ifndef _tests_spin_private_atomicprimitives_h
#define _tests_spin_private_atomicprimitives_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Spin
	{
		namespace Private
		{
			class atomicPrimitives : public CPPUNIT_NS::TestFixture
			{
				CPPUNIT_TEST_SUITE( atomicPrimitives );
				CPPUNIT_TEST(tryIncrement);
				CPPUNIT_TEST(tryAdd);
				CPPUNIT_TEST(tryFetchAndDecrement);
				CPPUNIT_TEST(tryAtomicSet);
				CPPUNIT_TEST_SUITE_END();

			public:
				virtual void setUp();
				virtual void tearDown();

			protected:
				void tryIncrement();
				void tryAdd();
				void tryFetchAndDecrement();
				void tryAtomicSet();
			private:
			};
		}
	}
}

#endif
