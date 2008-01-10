#include "Request.h"
#include <Damon/Request.h>
#include <Damon/Response.h>

namespace Tests
{
	namespace Damon
	{
		CPPUNIT_TEST_SUITE_REGISTRATION(Request);

		void Request::setUp()
		{ /* no-op */ }

		void Request::tearDown()
		{ /* no-op */ }

		void Request::tryEmptyRequest()
		{
			::Damon::Request request("http://www.google.com");
		}
	}
}
