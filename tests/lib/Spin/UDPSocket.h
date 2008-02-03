#ifndef _tests_spin_udpsocket_h
#define _tests_spin_udpsocket_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Spin
	{
		class UDPSocket : public CPPUNIT_NS::TestFixture
		{
			CPPUNIT_TEST_SUITE( UDPSocket );
			CPPUNIT_TEST( tryCreateInstance01 );
			CPPUNIT_TEST( tryCreateInstance02 );
			CPPUNIT_TEST( tryComm01 );
			CPPUNIT_TEST( tryComm02 );
			CPPUNIT_TEST( tryComm03 );
			CPPUNIT_TEST( tryPeek01 );
			CPPUNIT_TEST( tryTimeOut01 );
			CPPUNIT_TEST_SUITE_END();

		public:
			virtual void setUp();
			virtual void tearDown();

		protected:
			void tryCreateInstance01();
			void tryCreateInstance02();
			void tryComm01();
			void tryComm02();
			void tryComm03();
			void tryPeek01();
			void tryTimeOut01();

		private:
		};
	}
}

#endif
