#ifndef _tests_acari_attributes_h
#define _tests_acari_attributes_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Acari
	{
		class Attributes : public CPPUNIT_NS::TestFixture
		{
			CPPUNIT_TEST_SUITE( Attributes );
			CPPUNIT_TEST(tryAllocateOne);
			CPPUNIT_TEST(tryAllocateMany);
			CPPUNIT_TEST_SUITE_END();

		public:
			virtual void setUp();
			virtual void tearDown();

		protected:
			void tryAllocateOne();
			void tryAllocateMany();

		private:
		};
	}
}

#endif
