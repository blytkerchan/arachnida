#ifndef _tests_spin_listener_h
#define _tests_spin_listener_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Spin
	{
		class Listener : public CPPUNIT_NS::TestFixture
		{
			CPPUNIT_TEST_SUITE( Listener );
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
