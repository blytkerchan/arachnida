#include "parseURL.h"
#include <Damon/Private/parseURL.h>

namespace Tests
{
	namespace Damon
	{
		CPPUNIT_TEST_SUITE_REGISTRATION(parseURL);

		void parseURL::setUp()
		{ /* no-op */ }

		void parseURL::tearDown()
		{ /* no-op */ }

		void parseURL::tryURL01()
		{
			std::string protocol;
			std::string server;
			boost::uint16_t port;
			std::string resource;
			std::string username;
			std::string password;
			boost::tie(protocol, server, port, resource, username, password) = ::Damon::Private::parseURL("http://127.0.0.1/");
			CPPUNIT_ASSERT(protocol == "http");
			CPPUNIT_ASSERT(server == "127.0.0.1");
			CPPUNIT_ASSERT(port == 80);
			CPPUNIT_ASSERT(resource == "/");
			CPPUNIT_ASSERT(username == "");
			CPPUNIT_ASSERT(password == "");
		}

		void parseURL::tryURL02()
		{
			std::string protocol;
			std::string server;
			boost::uint16_t port;
			std::string resource;
			std::string username;
			std::string password;
			boost::tie(protocol, server, port, resource, username, password) = ::Damon::Private::parseURL("http://127.0.0.1");
			CPPUNIT_ASSERT(protocol == "http");
			CPPUNIT_ASSERT(server == "127.0.0.1");
			CPPUNIT_ASSERT(port == 80);
			CPPUNIT_ASSERT(resource == "/");
			CPPUNIT_ASSERT(username == "");
			CPPUNIT_ASSERT(password == "");
		}

		void parseURL::tryURL03()
		{
			std::string protocol;
			std::string server;
			boost::uint16_t port;
			std::string resource;
			std::string username;
			std::string password;
			boost::tie(protocol, server, port, resource, username, password) = ::Damon::Private::parseURL("127.0.0.1");
			CPPUNIT_ASSERT(protocol == "http");
			CPPUNIT_ASSERT(server == "127.0.0.1");
			CPPUNIT_ASSERT(port == 80);
			CPPUNIT_ASSERT(resource == "/");
			CPPUNIT_ASSERT(username == "");
			CPPUNIT_ASSERT(password == "");
		}

		void parseURL::tryURL04()
		{
			std::string protocol;
			std::string server;
			boost::uint16_t port;
			std::string resource;
			std::string username;
			std::string password;
			boost::tie(protocol, server, port, resource, username, password) = ::Damon::Private::parseURL("127.0.0.1:8080");
			CPPUNIT_ASSERT(protocol == "http");
			CPPUNIT_ASSERT(server == "127.0.0.1");
			CPPUNIT_ASSERT(port == 8080);
			CPPUNIT_ASSERT(resource == "/");
			CPPUNIT_ASSERT(username == "");
			CPPUNIT_ASSERT(password == "");
		}

		void parseURL::tryURL05()
		{
			std::string protocol;
			std::string server;
			boost::uint16_t port;
			std::string resource;
			std::string username;
			std::string password;
			boost::tie(protocol, server, port, resource, username, password) = ::Damon::Private::parseURL("https://127.0.0.1");
			CPPUNIT_ASSERT(protocol == "https");
			CPPUNIT_ASSERT(server == "127.0.0.1");
			CPPUNIT_ASSERT(port == 443);
			CPPUNIT_ASSERT(resource == "/");
			CPPUNIT_ASSERT(username == "");
			CPPUNIT_ASSERT(password == "");
		}

		void parseURL::tryURL06()
		{
			std::string protocol;
			std::string server;
			boost::uint16_t port;
			std::string resource;
			std::string username;
			std::string password;
			boost::tie(protocol, server, port, resource, username, password) = ::Damon::Private::parseURL("http://127.0.0.1:8080/");
			CPPUNIT_ASSERT(protocol == "http");
			CPPUNIT_ASSERT(server == "127.0.0.1");
			CPPUNIT_ASSERT(port == 8080);
			CPPUNIT_ASSERT(resource == "/");
			CPPUNIT_ASSERT(username == "");
			CPPUNIT_ASSERT(password == "");
		}

		void parseURL::tryURL07()
		{
			std::string protocol;
			std::string server;
			boost::uint16_t port;
			std::string resource;
			std::string username;
			std::string password;
			boost::tie(protocol, server, port, resource, username, password) = ::Damon::Private::parseURL("http://127.0.0.1:8080");
			CPPUNIT_ASSERT(protocol == "http");
			CPPUNIT_ASSERT(server == "127.0.0.1");
			CPPUNIT_ASSERT(port == 8080);
			CPPUNIT_ASSERT(resource == "/");
			CPPUNIT_ASSERT(username == "");
			CPPUNIT_ASSERT(password == "");
		}

		void parseURL::tryURL08()
		{
			std::string protocol;
			std::string server;
			boost::uint16_t port;
			std::string resource;
			std::string username;
			std::string password;
			boost::tie(protocol, server, port, resource, username, password) = ::Damon::Private::parseURL("http://user@127.0.0.1:8080");
			CPPUNIT_ASSERT(protocol == "http");
			CPPUNIT_ASSERT(server == "127.0.0.1");
			CPPUNIT_ASSERT(port == 8080);
			CPPUNIT_ASSERT(resource == "/");
			CPPUNIT_ASSERT(username == "user");
			CPPUNIT_ASSERT(password == "");
		}

		void parseURL::tryURL09()
		{
			std::string protocol;
			std::string server;
			boost::uint16_t port;
			std::string resource;
			std::string username;
			std::string password;
			boost::tie(protocol, server, port, resource, username, password) = ::Damon::Private::parseURL("http://127.0.0.1:8080");
			CPPUNIT_ASSERT(protocol == "http");
			CPPUNIT_ASSERT(server == "127.0.0.1");
			CPPUNIT_ASSERT(port == 8080);
			CPPUNIT_ASSERT(resource == "/");
			CPPUNIT_ASSERT(username == "");
			CPPUNIT_ASSERT(password == "");
		}

	}
}
