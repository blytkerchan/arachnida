#ifndef _tests_spin_private_connector_h
#define _tests_spin_private_connector_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Spin
	{
		namespace Private
		{
			class Connector : public CPPUNIT_NS::TestFixture
			{
				CPPUNIT_TEST_SUITE( Connector );
				CPPUNIT_TEST( tryCreateInstance );
				CPPUNIT_TEST( tryCreateHTTPConnectionToGoogle );
				CPPUNIT_TEST( tryCreateHTTPSConnectionToGoogle );
				CPPUNIT_TEST_SUITE_END();

			public:
				virtual void setUp();
				virtual void tearDown();

			protected:
				void tryCreateInstance();
				void tryCreateHTTPConnectionToGoogle();
				void tryCreateHTTPSConnectionToGoogle();

			private:
			};
		}
	}
}

#endif
