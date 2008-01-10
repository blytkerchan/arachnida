#ifndef _tests_spin_response_h
#define _tests_spin_response_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Damon
	{
		class Request : public CPPUNIT_NS::TestFixture
		{
			CPPUNIT_TEST_SUITE( Request );
			CPPUNIT_TEST( tryEmptyRequest );
			CPPUNIT_TEST_SUITE_END();

		public:
			virtual void setUp();
			virtual void tearDown();

		protected:
			void tryEmptyRequest();

		private:
		};
	}
}

#endif
