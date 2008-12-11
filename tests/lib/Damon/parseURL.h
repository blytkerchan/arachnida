#ifndef _tests_damon_parseurl_h
#define _tests_damon_parseurl_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Damon
	{
		class parseURL : public CPPUNIT_NS::TestFixture
		{
			CPPUNIT_TEST_SUITE( parseURL );
			CPPUNIT_TEST( tryURL01 );
			CPPUNIT_TEST( tryURL02 );
			CPPUNIT_TEST( tryURL03 );
			CPPUNIT_TEST( tryURL04 );
			CPPUNIT_TEST( tryURL05 );
			CPPUNIT_TEST( tryURL06 );
			CPPUNIT_TEST( tryURL07 );
			CPPUNIT_TEST( tryURL08 );
			CPPUNIT_TEST( tryURL09 );
			CPPUNIT_TEST_SUITE_END();

		public:
			virtual void setUp();
			virtual void tearDown();

		protected:
			void tryURL01();
			void tryURL02();
			void tryURL03();
			void tryURL04();
			void tryURL05();
			void tryURL06();
			void tryURL07();
			void tryURL08();
			void tryURL09();

		private:
		};
	}
}

#endif
