#ifndef _tests_spin_private_listener_h
#define _tests_spin_private_listener_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Spin
	{
		namespace Private
		{
			class Listener : public CPPUNIT_NS::TestFixture
			{
				CPPUNIT_TEST_SUITE( Listener );
				CPPUNIT_TEST_SUITE_END();

			public:
				virtual void setUp();
				virtual void tearDown();

			protected:
			private:
			};
		}
	}
}

#endif
