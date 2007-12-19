#include "Listener.h"
#include <Scorpion/Context.h>
#include <Spin/Listener.h>
#include <Spin/Connector.h>
#include <Spin/Details/Request.h>
#include <Spin/Handlers/NewConnectionHandler.h>
#include <Spin/Handlers/HTTPConnectionHandler.h>
#include <Spin/Handlers/HTTPRequestHandler.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <loki/ScopeGuard.h>
#if defined(_WIN32) && ! defined(__CYGWIN__)
#include <Windows.h>
#else
extern "C" {
#include <unistd.h>
}
#define Sleep(t) sleep((t) / 1000)
#endif

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
			::Scorpion::Context server_ssl_context(::Scorpion::Context::insecure_default_options__);
			server_ssl_context.setServerCertificateFilename(cert_path__);
			server_ssl_context.setPrivateKeyFilename(cert_path__);
			::Spin::Listener listener(server_ssl_context, 0, 4098);
			listener.setNewConnectionHandler(handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect(::Scorpion::Context(::Scorpion::Context::insecure_default_options__), "127.0.0.1", 4098));
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

		void Listener::tryAcceptWithHTTPHandler01()
		{
			::Spin::Listener listener(0, 4100);
			::Spin::Handlers::HTTPRequestHandler request_handler;
			::Spin::Handlers::HTTPConnectionHandler connection_handler(request_handler);
			listener.setNewConnectionHandler(connection_handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4100));
			connection_out.write(
				"POST / HTTP/1.1\r\n"
				"Content-Length: 8\r\n"
				"\r\n"
				"01234567"
				);
			boost::shared_ptr< ::Spin::Details::Request > request(request_handler.getNextRequest());
			CPPUNIT_ASSERT(request->method_ == "POST");
			CPPUNIT_ASSERT(request->url_ == "/");
			CPPUNIT_ASSERT(request->protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(request->body_.size() == 8);
			CPPUNIT_ASSERT(std::string(request->body_.begin(), request->body_.end()) == "01234567");
		}

		void Listener::tryAcceptWithHTTPHandler02()
		{
			::Spin::Listener listener(0, 4101);
			::Spin::Handlers::HTTPRequestHandler request_handler;
			::Spin::Handlers::HTTPConnectionHandler connection_handler(request_handler);
			listener.setNewConnectionHandler(connection_handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4101));
			connection_out.write(
				"GET / HTTP/1.1\r\n"
				"\r\n"
				);
			boost::shared_ptr< ::Spin::Details::Request > request(request_handler.getNextRequest());
			CPPUNIT_ASSERT(request->method_ == "GET");
			CPPUNIT_ASSERT(request->url_ == "/");
			CPPUNIT_ASSERT(request->protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(request->body_.empty());
		}

		void Listener::tryAcceptWithHTTPHandler03()
		{
			::Spin::Listener listener(0, 4102);
			::Spin::Handlers::HTTPRequestHandler request_handler;
			::Spin::Handlers::HTTPConnectionHandler connection_handler(request_handler);
			listener.setNewConnectionHandler(connection_handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4102));
			connection_out.write(
				"GET /index.html HTTP/1.1\r\n"
				"\r\n"
				"GET /images/logo.png HTTP/1.1\r\n"
				"\r\n"
				);
			boost::shared_ptr< ::Spin::Details::Request > request(request_handler.getNextRequest());
			CPPUNIT_ASSERT(request->method_ == "GET");
			CPPUNIT_ASSERT(request->url_ == "/index.html");
			CPPUNIT_ASSERT(request->protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(request->body_.empty());
			request = request_handler.getNextRequest();
			CPPUNIT_ASSERT(request->method_ == "GET");
			CPPUNIT_ASSERT(request->url_ == "/images/logo.png");
			CPPUNIT_ASSERT(request->protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(request->body_.empty());
		}

		void Listener::tryAcceptWithHTTPHandler04()
		{
			::Spin::Listener listener(0, 4103);
			::Spin::Handlers::HTTPRequestHandler request_handler;
			::Spin::Handlers::HTTPConnectionHandler connection_handler(request_handler);
			listener.setNewConnectionHandler(connection_handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4103));
			connection_out.write(
				"GET /index.html HTTP/1.1\r\n"
				"User-Agent: test\r\n"
				"Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n"
				"Accept-Language: en-us,en;q=0\r\n"
				"Accept-Encoding: gzip,deflate\r\n"
				"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
				"Keep-Alive: 300\r\n"
				"Connection: keep-alive\r\n"
				"\r\n"
				);
			boost::shared_ptr< ::Spin::Details::Request > request(request_handler.getNextRequest());
			CPPUNIT_ASSERT(request->method_ == "GET");
			CPPUNIT_ASSERT(request->url_ == "/index.html");
			CPPUNIT_ASSERT(request->protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(request->body_.empty());
			CPPUNIT_ASSERT(request->header_fields_.size() == 7);
			::Spin::Details::Request::HeaderFields::const_iterator where(request->header_fields_.begin());
			CPPUNIT_ASSERT(where->name_ == "User-Agent");
			CPPUNIT_ASSERT(where->value_ == "test");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept");
			CPPUNIT_ASSERT(where->value_ == "text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Language");
			CPPUNIT_ASSERT(where->value_ == "en-us,en;q=0");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Encoding");
			CPPUNIT_ASSERT(where->value_ == "gzip,deflate");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Charset");
			CPPUNIT_ASSERT(where->value_ == "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Keep-Alive");
			CPPUNIT_ASSERT(where->value_ == "300");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Connection");
			CPPUNIT_ASSERT(where->value_ == "keep-alive");
			++where;
		}

		void Listener::tryAcceptWithHTTPHandler05()
		{
			::Spin::Listener listener(0, 4104);
			::Spin::Handlers::HTTPRequestHandler request_handler;
			::Spin::Handlers::HTTPConnectionHandler connection_handler(request_handler);
			listener.setNewConnectionHandler(connection_handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4104));
			connection_out.write(
				"GET /index.html HTTP/1.1\r\n"
				"User-Agent: test\r\n"
				"Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n"
				"Accept-Language: en-us,en;q=0\r\n"
				"Accept-Encoding: gzip,deflate\r\n"
				);
			Sleep(1000);
			connection_out.write(
				"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
				"Keep-Alive: 300\r\n"
				"Connection: keep-alive\r\n"
				"\r\n"
				);
			boost::shared_ptr< ::Spin::Details::Request > request(request_handler.getNextRequest());
			CPPUNIT_ASSERT(request->method_ == "GET");
			CPPUNIT_ASSERT(request->url_ == "/index.html");
			CPPUNIT_ASSERT(request->protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(request->body_.empty());
			CPPUNIT_ASSERT(request->header_fields_.size() == 7);
			::Spin::Details::Request::HeaderFields::const_iterator where(request->header_fields_.begin());
			CPPUNIT_ASSERT(where->name_ == "User-Agent");
			CPPUNIT_ASSERT(where->value_ == "test");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept");
			CPPUNIT_ASSERT(where->value_ == "text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Language");
			CPPUNIT_ASSERT(where->value_ == "en-us,en;q=0");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Encoding");
			CPPUNIT_ASSERT(where->value_ == "gzip,deflate");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Charset");
			CPPUNIT_ASSERT(where->value_ == "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Keep-Alive");
			CPPUNIT_ASSERT(where->value_ == "300");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Connection");
			CPPUNIT_ASSERT(where->value_ == "keep-alive");
			++where;
		}

		void Listener::tryAcceptWithHTTPHandler06()
		{
			::Spin::Listener listener(0, 4105);
			::Spin::Handlers::HTTPRequestHandler request_handler;
			::Spin::Handlers::HTTPConnectionHandler connection_handler(request_handler);
			listener.setNewConnectionHandler(connection_handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4105));
			connection_out.write(
				"GET /index.html HTTP/1.1\r\n"
				"User-Agent: test\r\n"
				"Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n"
				"Accept-Language: en-us,en;q=0\r\n"
				"Accept-Encoding: gzip,deflate"
				);
			Sleep(1000);
			connection_out.write(
				"\r\n"
				"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
				"Keep-Alive: 300\r\n"
				"Connection: keep-alive\r\n"
				"\r\n"
				);
			boost::shared_ptr< ::Spin::Details::Request > request(request_handler.getNextRequest());
			CPPUNIT_ASSERT(request->method_ == "GET");
			CPPUNIT_ASSERT(request->url_ == "/index.html");
			CPPUNIT_ASSERT(request->protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(request->body_.empty());
			CPPUNIT_ASSERT(request->header_fields_.size() == 7);
			::Spin::Details::Request::HeaderFields::const_iterator where(request->header_fields_.begin());
			CPPUNIT_ASSERT(where->name_ == "User-Agent");
			CPPUNIT_ASSERT(where->value_ == "test");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept");
			CPPUNIT_ASSERT(where->value_ == "text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Language");
			CPPUNIT_ASSERT(where->value_ == "en-us,en;q=0");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Encoding");
			CPPUNIT_ASSERT(where->value_ == "gzip,deflate");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Charset");
			CPPUNIT_ASSERT(where->value_ == "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Keep-Alive");
			CPPUNIT_ASSERT(where->value_ == "300");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Connection");
			CPPUNIT_ASSERT(where->value_ == "keep-alive");
			++where;
		}

		void Listener::tryAcceptWithHTTPHandler07()
		{
			::Spin::Listener listener(0, 4106);
			::Spin::Handlers::HTTPRequestHandler request_handler;
			::Spin::Handlers::HTTPConnectionHandler connection_handler(request_handler);
			listener.setNewConnectionHandler(connection_handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4106));
			connection_out.write(
				"GET /index.html HTTP/1.1\r\n"
				"User-Agent: test\r\n"
				"Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n"
				"Accept-Language: en-us,en;q=0\r\n"
				"Accept-Encoding: gzip,"
				);
			Sleep(1000);
			connection_out.write(
				"deflate\r\n"
				"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
				"Keep-Alive: 300\r\n"
				"Connection: keep-alive\r\n"
				"\r\n"
				);
			boost::shared_ptr< ::Spin::Details::Request > request(request_handler.getNextRequest());
			CPPUNIT_ASSERT(request->method_ == "GET");
			CPPUNIT_ASSERT(request->url_ == "/index.html");
			CPPUNIT_ASSERT(request->protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(request->body_.empty());
			CPPUNIT_ASSERT(request->header_fields_.size() == 7);
			::Spin::Details::Request::HeaderFields::const_iterator where(request->header_fields_.begin());
			CPPUNIT_ASSERT(where->name_ == "User-Agent");
			CPPUNIT_ASSERT(where->value_ == "test");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept");
			CPPUNIT_ASSERT(where->value_ == "text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Language");
			CPPUNIT_ASSERT(where->value_ == "en-us,en;q=0");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Encoding");
			CPPUNIT_ASSERT(where->value_ == "gzip,deflate");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Charset");
			CPPUNIT_ASSERT(where->value_ == "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Keep-Alive");
			CPPUNIT_ASSERT(where->value_ == "300");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Connection");
			CPPUNIT_ASSERT(where->value_ == "keep-alive");
			++where;
		}

		void Listener::tryAcceptWithHTTPHandler08()
		{
			::Spin::Listener listener(0, 4107);
			::Spin::Handlers::HTTPRequestHandler request_handler;
			::Spin::Handlers::HTTPConnectionHandler connection_handler(request_handler);
			listener.setNewConnectionHandler(connection_handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4107));
			connection_out.write(
				"GET /index.html HTTP/1.1\r\n"
				"User-Agent: test\r\n"
				"Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n"
				"Accept-Language: en-us,en;q=0\r\n"
				"Accept-Encoding: gzip,\r\n"
				);
			Sleep(1000);
			connection_out.write(
				"\tdeflate\r\n"
				"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
				"Keep-Alive: 300\r\n"
				"Connection: keep-alive\r\n"
				"\r\n"
				);
			boost::shared_ptr< ::Spin::Details::Request > request(request_handler.getNextRequest());
			CPPUNIT_ASSERT(request->method_ == "GET");
			CPPUNIT_ASSERT(request->url_ == "/index.html");
			CPPUNIT_ASSERT(request->protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(request->body_.empty());
			CPPUNIT_ASSERT(request->header_fields_.size() == 7);
			::Spin::Details::Request::HeaderFields::const_iterator where(request->header_fields_.begin());
			CPPUNIT_ASSERT(where->name_ == "User-Agent");
			CPPUNIT_ASSERT(where->value_ == "test");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept");
			CPPUNIT_ASSERT(where->value_ == "text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Language");
			CPPUNIT_ASSERT(where->value_ == "en-us,en;q=0");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Encoding");
			CPPUNIT_ASSERT(where->value_ == "gzip,\r\n\tdeflate");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Charset");
			CPPUNIT_ASSERT(where->value_ == "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Keep-Alive");
			CPPUNIT_ASSERT(where->value_ == "300");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Connection");
			CPPUNIT_ASSERT(where->value_ == "keep-alive");
			++where;
		}

		void Listener::tryAcceptWithHTTPHandler09()
		{
			::Spin::Listener listener(0, 4107);
			::Spin::Handlers::HTTPRequestHandler request_handler;
			::Spin::Handlers::HTTPConnectionHandler connection_handler(request_handler);
			listener.setNewConnectionHandler(connection_handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4107));
			connection_out.write(
				"GET /index.html HTTP/1.1\r\n"
				"User-Agent: test\r\n"
				"Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n"
				"Accept-Language: en-us,en;q=0\r\n"
				"Accept-Encoding: gzip,\r\n"
				"\tdeflate\r\n"
				"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
				"Keep-Alive: 300\r\n"
				"Connection: keep-alive\r\n"
				);
			Sleep(1000);
			connection_out.write(
				"\r\n"
				);
			boost::shared_ptr< ::Spin::Details::Request > request(request_handler.getNextRequest());
			CPPUNIT_ASSERT(request->method_ == "GET");
			CPPUNIT_ASSERT(request->url_ == "/index.html");
			CPPUNIT_ASSERT(request->protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(request->body_.empty());
			CPPUNIT_ASSERT(request->header_fields_.size() == 7);
			::Spin::Details::Request::HeaderFields::const_iterator where(request->header_fields_.begin());
			CPPUNIT_ASSERT(where->name_ == "User-Agent");
			CPPUNIT_ASSERT(where->value_ == "test");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept");
			CPPUNIT_ASSERT(where->value_ == "text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Language");
			CPPUNIT_ASSERT(where->value_ == "en-us,en;q=0");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Encoding");
			CPPUNIT_ASSERT(where->value_ == "gzip,\r\n\tdeflate");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Charset");
			CPPUNIT_ASSERT(where->value_ == "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Keep-Alive");
			CPPUNIT_ASSERT(where->value_ == "300");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Connection");
			CPPUNIT_ASSERT(where->value_ == "keep-alive");
			++where;
		}

		void Listener::tryAcceptWithHTTPHandler10()
		{
			::Spin::Listener listener(0, 4108);
			::Spin::Handlers::HTTPRequestHandler request_handler;
			::Spin::Handlers::HTTPConnectionHandler connection_handler(request_handler);
			listener.setNewConnectionHandler(connection_handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4108));
			connection_out.write(
				"GET /index.html HTTP/1.1"
				);
			Sleep(1000);
			connection_out.write(
				"\r\n"
				"User-Agent: test\r\n"
				"Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n"
				"Accept-Language: en-us,en;q=0\r\n"
				"Accept-Encoding: gzip,\r\n"
				"\tdeflate\r\n"
				"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
				"Keep-Alive: 300\r\n"
				"Connection: keep-alive\r\n"
				"\r\n"
				);
			boost::shared_ptr< ::Spin::Details::Request > request(request_handler.getNextRequest());
			CPPUNIT_ASSERT(request->method_ == "GET");
			CPPUNIT_ASSERT(request->url_ == "/index.html");
			CPPUNIT_ASSERT(request->protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(request->body_.empty());
			CPPUNIT_ASSERT(request->header_fields_.size() == 7);
			::Spin::Details::Request::HeaderFields::const_iterator where(request->header_fields_.begin());
			CPPUNIT_ASSERT(where->name_ == "User-Agent");
			CPPUNIT_ASSERT(where->value_ == "test");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept");
			CPPUNIT_ASSERT(where->value_ == "text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Language");
			CPPUNIT_ASSERT(where->value_ == "en-us,en;q=0");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Encoding");
			CPPUNIT_ASSERT(where->value_ == "gzip,\r\n\tdeflate");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Charset");
			CPPUNIT_ASSERT(where->value_ == "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Keep-Alive");
			CPPUNIT_ASSERT(where->value_ == "300");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Connection");
			CPPUNIT_ASSERT(where->value_ == "keep-alive");
			++where;
		}

		void Listener::tryAcceptWithHTTPHandler11()
		{
			::Spin::Listener listener(0, 4109);
			::Spin::Handlers::HTTPRequestHandler request_handler;
			::Spin::Handlers::HTTPConnectionHandler connection_handler(request_handler);
			listener.setNewConnectionHandler(connection_handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4109));
			connection_out.write(
				"G"
				);
			Sleep(1000);
			connection_out.write(
				"ET /index"
				);
			Sleep(1000);
			connection_out.write(
				".html HTT"
				);
			Sleep(1000);
			connection_out.write(
				"P/1.1\r\n"
				"User-Agent: test\r\n"
				"Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n"
				"Accept-Language: en-us,en;q=0\r\n"
				"Accept-Encoding: gzip,\r\n"
				"\tdeflate\r\n"
				"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
				"Keep-Alive: 300\r\n"
				"Connection: keep-alive\r\n"
				"\r\n"
				);
			boost::shared_ptr< ::Spin::Details::Request > request(request_handler.getNextRequest());
			CPPUNIT_ASSERT(request->method_ == "GET");
			CPPUNIT_ASSERT(request->url_ == "/index.html");
			CPPUNIT_ASSERT(request->protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(request->body_.empty());
			CPPUNIT_ASSERT(request->header_fields_.size() == 7);
			::Spin::Details::Request::HeaderFields::const_iterator where(request->header_fields_.begin());
			CPPUNIT_ASSERT(where->name_ == "User-Agent");
			CPPUNIT_ASSERT(where->value_ == "test");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept");
			CPPUNIT_ASSERT(where->value_ == "text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Language");
			CPPUNIT_ASSERT(where->value_ == "en-us,en;q=0");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Encoding");
			CPPUNIT_ASSERT(where->value_ == "gzip,\r\n\tdeflate");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Charset");
			CPPUNIT_ASSERT(where->value_ == "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Keep-Alive");
			CPPUNIT_ASSERT(where->value_ == "300");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Connection");
			CPPUNIT_ASSERT(where->value_ == "keep-alive");
			++where;
		}

		void Listener::tryAcceptWithHTTPHandler12()
		{
			::Spin::Listener listener(0, 4110);
			::Spin::Handlers::HTTPRequestHandler request_handler;
			::Spin::Handlers::HTTPConnectionHandler connection_handler(request_handler);
			listener.setNewConnectionHandler(connection_handler);
			Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
			::Spin::Connection connection_out(::Spin::Connector::getInstance().connect("127.0.0.1", 4110));
			connection_out.write(
				"GET /index.html HTTP/1.1\r\n"
				"User-Agent: test\r\n"
				"Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n"
				"Accept-Language: en-us,en;q=0\r\n"
				"Accept-Encoding: gzip,\r\n"
				"\tdeflate\r\n"
				"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
				"Keep-Alive: 300\r\n"
				"Connection: keep-alive\r\n"
				"Content-Length: 10\r\n"
				"\r\n"
				);
			Sleep(1000);
			connection_out.write(
				"01234"
				);
			Sleep(1000);
			connection_out.write(
				"56789"
				);
			boost::shared_ptr< ::Spin::Details::Request > request(request_handler.getNextRequest());
			CPPUNIT_ASSERT(request->method_ == "GET");
			CPPUNIT_ASSERT(request->url_ == "/index.html");
			CPPUNIT_ASSERT(request->protocol_and_version_ == "HTTP/1.1");
			CPPUNIT_ASSERT(request->header_fields_.size() == 8);
			::Spin::Details::Request::HeaderFields::const_iterator where(request->header_fields_.begin());
			CPPUNIT_ASSERT(where->name_ == "User-Agent");
			CPPUNIT_ASSERT(where->value_ == "test");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept");
			CPPUNIT_ASSERT(where->value_ == "text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Language");
			CPPUNIT_ASSERT(where->value_ == "en-us,en;q=0");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Encoding");
			CPPUNIT_ASSERT(where->value_ == "gzip,\r\n\tdeflate");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Accept-Charset");
			CPPUNIT_ASSERT(where->value_ == "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Keep-Alive");
			CPPUNIT_ASSERT(where->value_ == "300");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Connection");
			CPPUNIT_ASSERT(where->value_ == "keep-alive");
			++where;
			CPPUNIT_ASSERT(where->name_ == "Content-Length");
			CPPUNIT_ASSERT(where->value_ == "10");
			++where;
			CPPUNIT_ASSERT(request->body_.size() == 10);
		}
	}
}
