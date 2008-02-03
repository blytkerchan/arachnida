#include "Connector.h"
#include <Spin/Connector.h>
#include <Spin/Connection.h>
#include <boost/tuple/tuple.hpp>
#include <Spin/Exceptions/Connection.h>
#include <Scorpion/Context.h>

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

			void Connector::tryCreateHTTPConnectionToLandheerCieslakDotCom()
			{
				boost::shared_ptr< ::Spin::Connection > connection(::Spin::Connector::getInstance().connect("www.landheer-cieslak.com", 80));
				CPPUNIT_ASSERT(!connection->usesSSL());
				std::size_t byte_count(0);
				int reason(0);
				boost::tie(byte_count, reason) = connection->write("GET / HTTP/1.0\n\n");
				CPPUNIT_ASSERT(byte_count == 16);
				CPPUNIT_ASSERT(reason == ::Spin::Connection::no_error__);
				std::vector< char > buffer;
				boost::tie(byte_count, reason) = connection->read(buffer);
				CPPUNIT_ASSERT(reason & ::Spin::Connection::should_retry__);
				buffer.resize(byte_count);
				std::cerr << std::string(buffer.begin(), buffer.end()) << std::endl;
				bool caught(false);
				try
				{
					while (1)
						connection->read(buffer);
				}
				catch (const ::Spin::Exceptions::Connection::ConnectionClosed &)
				{
					caught = true;
				}
				CPPUNIT_ASSERT(caught);
			}

			void Connector::tryCreateHTTPSConnectionToLandheerCieslakDotCom()
			{
				boost::shared_ptr< ::Spin::Connection > connection(::Spin::Connector::getInstance().connect(::Scorpion::Context(::Scorpion::Context::insecure_default_options__), "vlinder.landheer-cieslak.com", 443));
				CPPUNIT_ASSERT(connection->usesSSL());
				std::size_t byte_count(0);
				int reason(0);
				boost::tie(byte_count, reason) = connection->write("GET / HTTP/1.0\n\n");
				CPPUNIT_ASSERT(byte_count == 16);
				CPPUNIT_ASSERT(reason == ::Spin::Connection::no_error__);
				std::vector< char > buffer;
				boost::tie(byte_count, reason) = connection->read(buffer);
				CPPUNIT_ASSERT(reason & ::Spin::Connection::should_retry__);
				buffer.resize(byte_count);
				std::cerr << std::string(buffer.begin(), buffer.end()) << std::endl;
				bool caught(false);
				try
				{
					while (1)
						connection->read(buffer);
				}
				catch (const ::Spin::Exceptions::Connection::ConnectionClosed &)
				{
					caught = true;
				}
				CPPUNIT_ASSERT(caught);
			}
		}
	}
}
