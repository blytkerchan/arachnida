#ifndef _tests_damon_request_h
#define _tests_damon_request_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Damon
	{
		class Request : public CPPUNIT_NS::TestFixture
		{
			CPPUNIT_TEST_SUITE( Request );
			CPPUNIT_TEST( tryRequest01 );
			CPPUNIT_TEST( tryRequest02 );
			CPPUNIT_TEST( tryRequest03 );
			CPPUNIT_TEST( tryRequest04 );
			CPPUNIT_TEST_SUITE_END();

		public:
			virtual void setUp();
			virtual void tearDown();

		protected:
			void tryRequest01();
			void tryRequest02();
			void tryRequest03();
			void tryRequest04();

		private:
		};
	}
}

#endif
