#ifndef _tests_spin_response_h
#define _tests_spin_response_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Spin
	{
		class Response : public CPPUNIT_NS::TestFixture
		{
			CPPUNIT_TEST_SUITE( Response );
			CPPUNIT_TEST( tryEmptyResponse );
			CPPUNIT_TEST( tryResponseWithHeaders );
			CPPUNIT_TEST( tryResponseWithBody );
			CPPUNIT_TEST( tryResponseWithResponse );
			CPPUNIT_TEST_SUITE_END();

		public:
			virtual void setUp();
			virtual void tearDown();

		protected:
			void tryEmptyResponse();
			void tryResponseWithHeaders();
			void tryResponseWithBody();
			void tryResponseWithResponse();

		private:
		};
	}
}

#endif
