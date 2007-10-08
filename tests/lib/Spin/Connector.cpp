#include "Connector.h"
#include <Spin/Connector.h>
#include <Spin/Connection.h>
#include <boost/tuple/tuple.hpp>

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
				::Spin::Connection connection(::Spin::Connector::getInstance().connect("www.google.com", 80));
				CPPUNIT_ASSERT(!connection.usesSSL());
				std::size_t byte_count(0);
				int reason(0);
				boost::tie(byte_count, reason) = connection.write("GET / HTTP/1.0\n\n");
				CPPUNIT_ASSERT(byte_count == 16);
				CPPUNIT_ASSERT(reason == ::Spin::Connection::no_error__);
				std::vector< char > buffer;
				boost::tie(byte_count, reason) = connection.read(buffer);
				CPPUNIT_ASSERT(reason & ::Spin::Connection::should_retry__);
				buffer.resize(byte_count);
				// when we get here, Google will have closed the connection
				CPPUNIT_ASSERT_THROW(connection.read(buffer), std::runtime_error);
			}

			void Connector::tryCreateHTTPSConnectionToGoogle()
			{
				::Spin::Connection connection(::Spin::Connector::getInstance().connect("www.google.com", 443, true));
				CPPUNIT_ASSERT(connection.usesSSL());
				std::size_t byte_count(0);
				int reason(0);
				boost::tie(byte_count, reason) = connection.write("GET / HTTP/1.0\n\n");
				CPPUNIT_ASSERT(byte_count == 16);
				CPPUNIT_ASSERT(reason == ::Spin::Connection::no_error__);
				std::vector< char > buffer;
				boost::tie(byte_count, reason) = connection.read(buffer);
				CPPUNIT_ASSERT(reason & ::Spin::Connection::should_retry__);
				buffer.resize(byte_count);
				// when we get here, Google will have closed the connection
				CPPUNIT_ASSERT_THROW(connection.read(buffer), std::runtime_error);
			}
		}
	}
}
