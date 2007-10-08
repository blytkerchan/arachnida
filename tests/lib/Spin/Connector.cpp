#include "Connector.h"
#include <Spin/Connector.h>

namespace Tests
{
	namespace Spin
	{
		namespace Private
		{
			CPPUNIT_TEST_SUITE_REGISTRATION(Connector);

			void Connector::setUp()
			{ /* no-op */ }

			void Connector::tearDown()
			{ /* no-op */ }

			void Connector::tryCreateInstance()
			{
				::Spin::Connector::getInstance();
			}

			void Connector::tryCreateHTTPConnectionToGoogle()
			{
				::Spin::Connector::getInstance().connect("www.google.com", 80);
			}

			void Connector::tryCreateHTTPSConnectionToGoogle()
			{
				::Spin::Connector::getInstance().connect("www.google.com", 443, true);
			}
		}
	}
}
