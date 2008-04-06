#include "Request.h"
#include <Damon/Request.h>
#include <Damon/Response.h>
#include <Spin/Exceptions/Connection/ConnectionClosed.h>

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

		void Request::tryRequest03()
		{
			std::vector< ::Damon::Request > requests;
			requests.push_back(::Damon::Request("http://127.0.0.1/send_leading_whitespace"));
			requests.push_back(::Damon::Request("http://127.0.0.1/"));
			requests.push_back(::Damon::Request("http://127.0.0.1/index.html"));
			requests.push_back(::Damon::Request("http://127.0.0.1/with space"));

			std::vector< ::Damon::Response > responses(send(requests));
			CPPUNIT_ASSERT(responses.size() == requests.size());
			CPPUNIT_ASSERT(responses[0].protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(responses[0].status_code_ == 302);
			CPPUNIT_ASSERT(responses[0].reason_string_ == "Found");
			CPPUNIT_ASSERT(responses[0].header_fields_.size() == 2);
			CPPUNIT_ASSERT(responses[0].header_fields_[0].name_ == "Location");
			CPPUNIT_ASSERT(responses[0].header_fields_[0].value_ == "/index.html");
			CPPUNIT_ASSERT(responses[0].header_fields_[1].name_ == "Content-Length");
			CPPUNIT_ASSERT(responses[0].header_fields_[1].value_ == "0");
			CPPUNIT_ASSERT(responses[0].body_.empty());
			CPPUNIT_ASSERT(responses[1].protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(responses[1].status_code_ == 302);
			CPPUNIT_ASSERT(responses[1].reason_string_ == "Found");
			CPPUNIT_ASSERT(responses[1].header_fields_.size() == 2);
			CPPUNIT_ASSERT(responses[1].header_fields_[0].name_ == "Location");
			CPPUNIT_ASSERT(responses[1].header_fields_[0].value_ == "/index.html");
			CPPUNIT_ASSERT(responses[1].header_fields_[1].name_ == "Content-Length");
			CPPUNIT_ASSERT(responses[1].header_fields_[1].value_ == "0");
			CPPUNIT_ASSERT(responses[1].body_.empty());
			CPPUNIT_ASSERT(responses[2].protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(responses[2].status_code_ == 200);
			CPPUNIT_ASSERT(responses[2].reason_string_ == "OK");
			CPPUNIT_ASSERT(responses[2].header_fields_.size() == 2);
			CPPUNIT_ASSERT(responses[2].header_fields_[0].name_ == "Content-Type");
			CPPUNIT_ASSERT(responses[2].header_fields_[0].value_ == "text/html");
			CPPUNIT_ASSERT(responses[2].header_fields_[1].name_ == "Content-Length");
			CPPUNIT_ASSERT(responses[2].header_fields_[1].value_ == "46");
			CPPUNIT_ASSERT(responses[2].body_.size() == 46);
			CPPUNIT_ASSERT(responses[3].protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(responses[3].status_code_ == 200);
			CPPUNIT_ASSERT(responses[3].reason_string_ == "OK");
			CPPUNIT_ASSERT(responses[3].header_fields_.size() == 2);
			CPPUNIT_ASSERT(responses[3].header_fields_[0].name_ == "Content-Type");
			CPPUNIT_ASSERT(responses[3].header_fields_[0].value_ == "text/html");
			CPPUNIT_ASSERT(responses[3].header_fields_[1].name_ == "Content-Length");
			CPPUNIT_ASSERT(responses[3].header_fields_[1].value_ == "46");
			CPPUNIT_ASSERT(responses[3].body_.size() == 46);
		}

		void Request::tryRequest04()
		{
			::Damon::Request request("http://127.0.0.1/close_connection");
			CPPUNIT_ASSERT_THROW(send(request), ::Spin::Exceptions::Connection::ConnectionClosed);
		}
	}
}
