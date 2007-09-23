#include "Server.h"
#include <Spin/Server.h>
#include <Spin/Handlers/NewConnectionHandler.h>
#include <Spin/Private/yield.h>
#include <Spin/Private/Logger.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <loki/ScopeGuard.h>

namespace
{
	class Log : public ::Spin::Private::Logger
	{
	public :
		Log()
		{
			if (getInstance() != 0)
				throw std::logic_error("This is not the only instance of a logger!");
			else
			{ /* no-op - all is well */ }
			setInstance(this);
		}

		~Log()
		{
			if (getInstance() == this)
				setInstance(0);
			else
			{ /* no-op */ }
		}

		/*virtual */void debug_(const std::string & component, const std::string & message, const std::string & aux)
		{
			std::cerr << component << ": \"" << message << " - " << aux << "\"" << std::endl;
		}

		/*virtual */void warning_(const std::string & component, const std::string & message, const std::string & aux)
		{
			std::cerr << component << ": \"" << message << " - " << aux << "\"" << std::endl;
		}

		/*virtual */void error_(const std::string & component, const std::string & message, const std::string & aux)
		{
			std::cerr << component << ": \"" << message << " - " << aux << "\"" << std::endl;
		}

		/*virtual */void fatalError_(const std::string & component, const std::string & message, const std::string & aux)
		{
			std::cerr << component << ": \"" << message << " - " << aux << "\"" << std::endl;
		}
	};

	struct ConnectionHandler : ::Spin::Handlers::NewConnectionHandler
	{
		ConnectionHandler()
			: called_(false)
		{ /* no-op */ }

		/*virtual */void handleNewConnection(std::auto_ptr< ACE_SOCK_Stream > & new_connection)/* = 0;*/
		{
			called_ = true;
		}

		bool called_;
	};
}
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
			Log log;

			::Spin::Server server("127.0.0.1", 4096);
		}

		void Server::tryCreateInstance02()
		{
			using ::Spin::Private::yield;
			Log log;

			::Spin::Server server("127.0.0.1", 8798);
			ConnectionHandler connection_handler;
			server.setNewConnectionHandler(&connection_handler);
			Loki::ScopeGuard connection_handler_guard = Loki::MakeObjGuard(server, &::Spin::Server::setNewConnectionHandler, (ConnectionHandler*)0);

			// create a connection - this should be moved to a separate function within the library
			ACE_SOCK_Connector connector;
			ACE_SOCK_Stream connection;
			ACE_INET_Addr address(8798, "127.0.0.1");
			int rv(connector.connect(connection, address));
			CPPUNIT_ASSERT(rv == 0);
			yield(true);
			CPPUNIT_ASSERT(connection_handler.called_);
		}
	}
}
