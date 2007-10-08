#include "Listener.h"
#include <Spin/Listener.h>
#include <Spin/Connector.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

extern boost::filesystem::path cert_path__;

namespace Tests
{
	namespace Spin
	{
		CPPUNIT_TEST_SUITE_REGISTRATION(Listener);

		void Listener::setUp()
		{ /* no-op */ }

		void Listener::tearDown()
		{ /* no-op */ }

		void Listener::tryCreateInstance()
		{
			::Spin::Listener listener(0, 4096);
		}

		void Listener::tryAccept()
		{
			::Spin::Listener listener(0, 4097);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4097));
			::Spin::Connection accepted_connection(listener.accept());
			connection_out.write("Hello, world!");
			std::vector< char > buffer;
			std::pair< std::size_t, int > results(accepted_connection.read(buffer));
			buffer.resize(results.first);
			CPPUNIT_ASSERT(std::string(buffer.begin(), buffer.end()) == "Hello, world!");
		}

		void Listener::tryAcceptSSL()
		{
			CPPUNIT_ASSERT(!cert_path__.empty());
			CPPUNIT_ASSERT(boost::filesystem::exists(cert_path__));
			::Spin::Listener listener(cert_path__, 0, 4098);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4098, true));
			::Spin::Connection accepted_connection(listener.accept());
			connection_out.write("Hello, world!");
			std::vector< char > buffer;
			std::pair< std::size_t, int > results(accepted_connection.read(buffer));
			buffer.resize(results.first);
			CPPUNIT_ASSERT(std::string(buffer.begin(), buffer.end()) == "Hello, world!");
		}
	}
}
