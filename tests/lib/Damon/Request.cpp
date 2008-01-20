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

		void Request::tryRequest01()
		{
			::Damon::Request request("http://127.0.0.1/");
			::Damon::Response response(send(request));
			CPPUNIT_ASSERT(response.protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(response.status_code_ == 302);
			CPPUNIT_ASSERT(response.reason_string_ == "Found");
			CPPUNIT_ASSERT(response.header_fields_.size() == 2);
			CPPUNIT_ASSERT(response.header_fields_[0].name_ == "Location");
			CPPUNIT_ASSERT(response.header_fields_[0].value_ == "/index.html");
			CPPUNIT_ASSERT(response.header_fields_[1].name_ == "Content-Length");
			CPPUNIT_ASSERT(response.header_fields_[1].value_ == "0");
			CPPUNIT_ASSERT(response.body_.empty());
		}

		void Request::tryRequest02()
		{
			::Damon::Request request("http://127.0.0.1/send_leading_whitespace");
			::Damon::Response response(send(request));
			CPPUNIT_ASSERT(response.protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(response.status_code_ == 302);
			CPPUNIT_ASSERT(response.reason_string_ == "Found");
			CPPUNIT_ASSERT(response.header_fields_.size() == 2);
			CPPUNIT_ASSERT(response.header_fields_[0].name_ == "Location");
			CPPUNIT_ASSERT(response.header_fields_[0].value_ == "/index.html");
			CPPUNIT_ASSERT(response.header_fields_[1].name_ == "Content-Length");
			CPPUNIT_ASSERT(response.header_fields_[1].value_ == "0");
			CPPUNIT_ASSERT(response.body_.empty());
		}
	}
}
