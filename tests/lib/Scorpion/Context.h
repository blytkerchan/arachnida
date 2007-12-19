#ifndef _tests_scorpion_context_h
#define _tests_scorpion_context_h

#include <cppunit/extensions/HelperMacros.h>

namespace Tests
{
	namespace Scorpion
	{
		class Context : public CPPUNIT_NS::TestFixture
		{
			CPPUNIT_TEST_SUITE( Context );
			CPPUNIT_TEST( tryCreateInstance );
			CPPUNIT_TEST( tryCopy );
			CPPUNIT_TEST( tryAssign );
			CPPUNIT_TEST( tryAssign );
			CPPUNIT_TEST( trySetCertificateName );
			CPPUNIT_TEST( trySetCertificateNameAndCopy );
			CPPUNIT_TEST( trySetCertificateNameAndAssign );
			CPPUNIT_TEST( trySetCertificateNameAndSwap );
			CPPUNIT_TEST( tryGetContext );
			CPPUNIT_TEST( tryGetContextAndCopy );
			CPPUNIT_TEST( tryGetContextAndCopyOnWrite01 );
			CPPUNIT_TEST( tryGetContextAndCopyOnWrite02 );
			CPPUNIT_TEST_SUITE_END();

		public:
			virtual void setUp();
			virtual void tearDown();

		protected:
			void tryCreateInstance();
			void tryCopy();
			void tryAssign();
			void trySetCertificateName();
			void trySetCertificateNameAndCopy();
			void trySetCertificateNameAndAssign();
			void trySetCertificateNameAndSwap();
			void tryGetContext();
			void tryGetContextAndCopy();
			void tryGetContextAndCopyOnWrite01();
			void tryGetContextAndCopyOnWrite02();

		private:
		};
	}
}

#endif
