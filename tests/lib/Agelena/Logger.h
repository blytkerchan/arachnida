#ifndef _tests_agelena_logger_h
#define _tests_agelena_logger_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Agelena
	{
		class Logger : public CPPUNIT_NS::TestFixture
		{
			CPPUNIT_TEST_SUITE( Logger );
			CPPUNIT_TEST( tryCreateInstance );
			CPPUNIT_TEST_SUITE_END();

		public:
			virtual void setUp();
			virtual void tearDown();

		protected:
			void tryCreateInstance();

		private:
		};
	}
}

#endif
