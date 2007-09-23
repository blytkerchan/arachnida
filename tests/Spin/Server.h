#ifndef _tests_spin_private_server_h
#define _tests_spin_private_server_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Spin
	{
		class Server : public CPPUNIT_NS::TestFixture
		{
			CPPUNIT_TEST_SUITE( Server );
			CPPUNIT_TEST( tryCreateInstance01 );
			CPPUNIT_TEST( tryCreateInstance02 );
			CPPUNIT_TEST_SUITE_END();

		public:
			virtual void setUp();
			virtual void tearDown();

		protected:
			void tryCreateInstance01();
			void tryCreateInstance02();

		private:
		};
	}
}

#endif
