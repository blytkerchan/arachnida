#ifndef _tests_spin_private_connection_h
#define _tests_spin_private_connection_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Spin
	{
		namespace Private
		{
			class Connection : public CPPUNIT_NS::TestFixture
			{
				CPPUNIT_TEST_SUITE( Connection );
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
