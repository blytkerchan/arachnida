#include "Server.h"
#include <Spin/Server.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
namespace Tests
{
	namespace Spin
	{
		CPPUNIT_TEST_SUITE_REGISTRATION(Server);

		void Server::setUp()
		{ /* no-op */ }

		void Server::tearDown()
		{ /* no-op */ }

		void Server::tryCreateInstance01()
		{
			::Spin::Server server("127.0.0.1", 4096);
		}

		void Server::tryCreateInstance02()
		{
			::Spin::Server server("127.0.0.1", 4096);
			// create a connection - this should be moved to a separate function within the library
			ACE_SOCK_Connector connector;
			ACE_SOCK_Stream connection;
			ACE_INET_Addr address(4096, "127.0.0.1");
			int rv(connector.connect(connection, address));
			CPPUNIT_ASSERT(rv == 0);
		}
	}
}
