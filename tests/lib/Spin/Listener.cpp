#include "Listener.h"
#include <Spin/Listener.h>
#include <Spin/Connector.h>
#include <Spin/Handlers/NewConnectionHandler.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <loki/ScopeGuard.h>
#include <Windows.h>

extern boost::filesystem::path cert_path__;

namespace Tests
{
	namespace Spin
	{
		namespace 
		{
			class NewConnectionHandler : public ::Spin::Handlers::NewConnectionHandler
			{
			public :
				::Spin::Connection getConnection()
				{
					CPPUNIT_ASSERT(connection_.get());
					return ::Spin::Connection(*connection_);
				}

			protected :
				/*virtual */void handleNewConnection(const ::Spin::Connection & connection)
				{
					connection_.reset(new ::Spin::Connection(connection));
				}

			private :
				std::auto_ptr< ::Spin::Connection > connection_;
			};

			class NewConnectionHandler2 : public ::Spin::Handlers::NewConnectionHandler
			{
			public :
				NewConnectionHandler2(::Spin::Listener & listener)
					: listener_(listener)
				{ /* no-op */ }

				::Spin::Connection getConnection()
				{
					CPPUNIT_ASSERT(connection_.get());
					return ::Spin::Connection(*connection_);
				}

			protected :
				/*virtual */void handleNewConnection(const ::Spin::Connection & connection)
				{
					connection_.reset(new ::Spin::Connection(connection));
					listener_.clearNewConnectionHandler();
				}

			private :
				::Spin::Listener & listener_;
				std::auto_ptr< ::Spin::Connection > connection_;
			};
		}

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
			NewConnectionHandler handler;

			CPPUNIT_ASSERT(!cert_path__.empty());
			CPPUNIT_ASSERT(boost::filesystem::exists(cert_path__));
			::Spin::Listener listener(cert_path__, 0, 4098);
			listener.setNewConnectionHandler(handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4098, true));
			::Spin::Connection accepted_connection(handler.getConnection());
			connection_out.write("Hello, world!");
			std::vector< char > buffer;
			std::pair< std::size_t, int > results(accepted_connection.read(buffer));
			buffer.resize(results.first);
			CPPUNIT_ASSERT(std::string(buffer.begin(), buffer.end()) == "Hello, world!");
		}

		void Listener::tryAsyncAcceptAndDetach()
		{
			::Spin::Listener listener(0, 4099);
			NewConnectionHandler2 handler(listener);
			listener.setNewConnectionHandler(handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4099));
			Sleep(1000);
			::Spin::Connection accepted_connection(handler.getConnection());
			connection_out.write("Hello, world!");
			std::vector< char > buffer;
			std::pair< std::size_t, int > results(accepted_connection.read(buffer));
			buffer.resize(results.first);
			CPPUNIT_ASSERT(std::string(buffer.begin(), buffer.end()) == "Hello, world!");
		}
	}
}
