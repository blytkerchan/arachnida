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
			CPPUNIT_TEST( tryAcceptWithHTTPHandler04 );
			CPPUNIT_TEST( tryAcceptWithHTTPHandler05 );
			CPPUNIT_TEST( tryAcceptWithHTTPHandler06 );
			CPPUNIT_TEST( tryAcceptWithHTTPHandler07 );
			CPPUNIT_TEST( tryAcceptWithHTTPHandler08 );
			CPPUNIT_TEST( tryAcceptWithHTTPHandler09 );
			CPPUNIT_TEST( tryAcceptWithHTTPHandler10 );
			CPPUNIT_TEST( tryAcceptWithHTTPHandler11 );
			CPPUNIT_TEST( tryAcceptWithHTTPHandler12 );
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
			void tryAcceptWithHTTPHandler04();
			void tryAcceptWithHTTPHandler05();
			void tryAcceptWithHTTPHandler06();
			void tryAcceptWithHTTPHandler07();
			void tryAcceptWithHTTPHandler08();
			void tryAcceptWithHTTPHandler09();
			void tryAcceptWithHTTPHandler10();
			void tryAcceptWithHTTPHandler11();
			void tryAcceptWithHTTPHandler12();

		private:
		};
	}
}

#endif
