#include "Server.h"
#include <Spin/Server.h>

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
			// create a connection here
		}
	}
}
