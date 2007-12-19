#ifndef _tests_acari_bakerycounter_h
#define _tests_acari_bakerycounter_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Acari
	{
		class BakeryCounter : public CPPUNIT_NS::TestFixture
		{
			CPPUNIT_TEST_SUITE( BakeryCounter );
			CPPUNIT_TEST(tryIncrementClient);
			CPPUNIT_TEST(tryIncrementBaker);
			CPPUNIT_TEST_SUITE_END();

		public:
			virtual void setUp();
			virtual void tearDown();

		protected:
			void tryIncrementClient();
			void tryIncrementBaker();
		private:
		};
	}
}

#endif
