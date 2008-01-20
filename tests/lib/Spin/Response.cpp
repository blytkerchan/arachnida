#include "Response.h"
#include <Spin/Details/Response.h>

namespace Tests
{
	namespace Spin
	{
		CPPUNIT_TEST_SUITE_REGISTRATION(Response);

		void Response::setUp()
		{ /* no-op */ }

		void Response::tearDown()
		{ /* no-op */ }

		void Response::tryEmptyResponse()
		{
			::Spin::Details::Response response("HTTP/1.1", ::Spin::Details::Response::ok__);
			std::vector< char > _serialized(serialize(response));
			std::string serialized(_serialized.begin(), _serialized.end());
			CPPUNIT_ASSERT(serialized == "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");
		}

		void Response::tryResponseWithHeaders()
		{
			::Spin::Details::Response response("HTTP/1.1", ::Spin::Details::Response::ok__);
			response("Test", "test");
			std::vector< char > _serialized(serialize(response));
			std::string serialized(_serialized.begin(), _serialized.end());
			CPPUNIT_ASSERT(serialized == "HTTP/1.1 200 OK\r\nTest: test\r\nContent-Length: 0\r\n\r\n");
		}

		void Response::tryResponseWithBody()
		{
			::Spin::Details::Response response("HTTP/1.1", ::Spin::Details::Response::ok__);
			response("Test");
			std::vector< char > _serialized(serialize(response));
			std::string serialized(_serialized.begin(), _serialized.end());
			CPPUNIT_ASSERT(serialized == "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nTest");
		}

		void Response::tryResponseWithResponse()
		{
			::Spin::Details::Response response("HTTP/1.1", ::Spin::Details::Response::ok__);
			response("Test")(response);
			std::vector< char > _serialized(serialize(response));
			std::string serialized(_serialized.begin(), _serialized.end());
			CPPUNIT_ASSERT(serialized == "HTTP/1.1 200 OK\r\nContent-Length: 42\r\n\r\nHTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nTest");
		}
	}
}
