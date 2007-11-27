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
			CPPUNIT_TEST( tryAccept );
			CPPUNIT_TEST( tryAcceptSSL );
			CPPUNIT_TEST( tryAsyncAcceptAndDetach );
			CPPUNIT_TEST( tryAcceptWithHTTPHandler01 );
			CPPUNIT_TEST( tryAcceptWithHTTPHandler02 );
			CPPUNIT_TEST( tryAcceptWithHTTPHandler03 );
			CPPUNIT_TEST_SUITE_END();

		public:
			virtual void setUp();
			virtual void tearDown();

		protected:
			void tryCreateInstance();
			void tryAccept();
			void tryAcceptSSL();
			void tryAsyncAcceptAndDetach();
			void tryAcceptWithHTTPHandler01();
			void tryAcceptWithHTTPHandler02();
			void tryAcceptWithHTTPHandler03();

		private:
		};
	}
}

#endif
