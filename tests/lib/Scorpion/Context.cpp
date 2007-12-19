#include "Context.h"
#include <Scorpion/Context.h>

namespace Tests
{
	namespace Scorpion
	{
		CPPUNIT_TEST_SUITE_REGISTRATION(Context);

		void Context::setUp()
		{ /* no-op */ }

		void Context::tearDown()
		{ /* no-op */ }

		void Context::tryCreateInstance()
		{
			::Scorpion::Context context;
		}

		void Context::tryCopy()
		{
			::Scorpion::Context context1;
			::Scorpion::Context context2(context1);
		}

		void Context::tryAssign()
		{
			::Scorpion::Context context1;
			::Scorpion::Context context2;

			context1 = context2;
		}

		void Context::trySetCertificateName()
		{
			::Scorpion::Context context;
			context.setServerCertificateFilename("Some/valid/name.pem");
			CPPUNIT_ASSERT(context.getServerCertificateFilename() == "Some/valid/name.pem");
		}

		void Context::trySetCertificateNameAndCopy()
		{
			::Scorpion::Context context;
			context.setServerCertificateFilename("Some/valid/name.pem");
			CPPUNIT_ASSERT(context.getServerCertificateFilename() == "Some/valid/name.pem");
			::Scorpion::Context context2(context);
			CPPUNIT_ASSERT(context2.getServerCertificateFilename() == "Some/valid/name.pem");
		}

		void Context::trySetCertificateNameAndAssign()
		{
			::Scorpion::Context context;
			context.setServerCertificateFilename("Some/valid/name.pem");
			CPPUNIT_ASSERT(context.getServerCertificateFilename() == "Some/valid/name.pem");
			::Scorpion::Context context2;
			context2 = context;
			CPPUNIT_ASSERT(context2.getServerCertificateFilename() == "Some/valid/name.pem");
		}

		void Context::trySetCertificateNameAndSwap()
		{
			::Scorpion::Context context;
			context.setServerCertificateFilename("Some/valid/name.pem");
			CPPUNIT_ASSERT(context.getServerCertificateFilename() == "Some/valid/name.pem");
			::Scorpion::Context context2;
			context2.swap(context);
			CPPUNIT_ASSERT(context2.getServerCertificateFilename() == "Some/valid/name.pem");
		}

		void Context::tryGetContext()
		{
			::Scorpion::Context context;
			CPPUNIT_ASSERT(context.getContext() != 0);
		}
		void Context::tryGetContextAndCopy()
		{
			::Scorpion::Context context;
			CPPUNIT_ASSERT(context.getContext() != 0);
			::Scorpion::Context context2(context);
			CPPUNIT_ASSERT(context.getContext() == context2.getContext());
		}

		void Context::tryGetContextAndCopyOnWrite01()
		{
			::Scorpion::Context context;
			CPPUNIT_ASSERT(context.getContext() != 0);
			::Scorpion::Context context2(context);
			CPPUNIT_ASSERT(context.getContext() == context2.getContext());
			context2.setFlags(::Scorpion::Context::use_sslv3__);
			CPPUNIT_ASSERT(context2.getContext() != 0);
			CPPUNIT_ASSERT(context.getContext() != context2.getContext());
		}

		void Context::tryGetContextAndCopyOnWrite02()
		{
			::Scorpion::Context context;
			CPPUNIT_ASSERT(context.getContext() != 0);
			::Scorpion::Context context2(context);
			CPPUNIT_ASSERT(context.getContext() == context2.getContext());
			context2.setServerCertificateFilename(boost::filesystem::path());
			CPPUNIT_ASSERT(context2.getContext() != 0);
			CPPUNIT_ASSERT(context.getContext() != context2.getContext());
		}
	}
}
