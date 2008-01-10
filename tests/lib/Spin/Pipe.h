#ifndef _tests_spin_private_pipe_h
#define _tests_spin_private_pipe_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Spin
	{
		namespace Private
		{
			class Pipe : public CPPUNIT_NS::TestFixture
			{
				CPPUNIT_TEST_SUITE( Pipe );
				CPPUNIT_TEST(tryCreateInstance);
				CPPUNIT_TEST(tryReadWrite);
				CPPUNIT_TEST_SUITE_END();

			public:
				virtual void setUp();
				virtual void tearDown();

			protected:
				void tryCreateInstance();
				void tryReadWrite();

			private:
			};
		}
	}
}

#endif
