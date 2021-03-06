#include <iostream>
#include <Spin/Listener.h>
#include <Spin/Details/Request.h>
#include <Spin/Details/Response.h>
#include <Spin/Handlers/NewConnectionHandler.h>
#include <Spin/Handlers/HTTPConnectionHandler.h>
#include <Spin/Handlers/HTTPRequestHandler.h>
#include <loki/ScopeGuard.h>
#include <Agelena/Loggers/DefaultWindowsLogger.h>

int main()
{
	Agelena::Loggers::DefaultWindowsLogger logger;
	Spin::Listener listener(0, 80);
	::Spin::Handlers::HTTPRequestHandler request_handler;
	::Spin::Handlers::HTTPConnectionHandler connection_handler(request_handler);
	listener.setNewConnectionHandler(connection_handler);
	Loki::ScopeGuard attachment_guard = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
	(void)attachment_guard;
	while (1)
	{
		boost::shared_ptr< ::Spin::Details::Request > request(request_handler.getNextRequest());
		boost::shared_ptr< Spin::Connection > connection(request->connection_.lock());
		if (connection) try
		{
			// we handle only GET requests - anything else and we respond with 501
			if (request->method_ == "GET")
			{
				if (request->url_ == "/")
				{
					connection->write(
						Spin::Details::Response(request->protocol_and_version_, Spin::Details::Response::found__)
							("Location", "/index.html")
						);
				}
				else if (request->url_ == "/index.html")
				{
					connection->write(
						Spin::Details::Response(request->protocol_and_version_, Spin::Details::Response::ok__)
						("Content-Type", "text/html")
						("<html><body><p>Hello, world!</p></body></html>")
						);
				}
				else if (request->url_ == "/with%20space")
				{
					connection->write(
						Spin::Details::Response(request->protocol_and_version_, Spin::Details::Response::ok__)
						("Content-Type", "text/html")
						("<html><body><p>Hello, space!</p></body></html>")
						);
				}
				else if (request->url_ == "/send_leading_whitespace")
				{
					connection->write("    \r\n\n\n\n\n\r          \t\t\t    ");
					connection->write(
						Spin::Details::Response(request->protocol_and_version_, Spin::Details::Response::found__)
						("Location", "/index.html")
						);
				}
				else if (request->url_ == "/close_connection")
				{
					connection->close();
				}
				else
				{
					connection->write(
						Spin::Details::Response(request->protocol_and_version_, Spin::Details::Response::not_found__)
					);
				}
			}
			else
			{
				connection->write(
					Spin::Details::Response(request->protocol_and_version_, Spin::Details::Response::not_implemented__)
				);
			}
		}
		catch (const std::exception & e)
		{
			std::cerr << e.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "Some unknown error occured" << std::endl;
		}
		else
		{ /* client hung up */ }
	}
}
/////