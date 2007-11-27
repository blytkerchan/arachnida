#include <Spin/Listener.h>
#include <Spin/Details/Request.h>
#include <Spin/Handlers/NewConnectionHandler.h>
#include <Spin/Handlers/HTTPConnectionHandler.h>
#include <Spin/Handlers/HTTPRequestHandler.h>
#include <loki/ScopeGuard.h>

int main()
{
	Spin::Listener listener(0, 80);
	::Spin::Handlers::HTTPRequestHandler request_handler;
	::Spin::Handlers::HTTPConnectionHandler connection_handler(request_handler);
	listener.setNewConnectionHandler(connection_handler);
	Loki::ScopeGuard attachment_handler = Loki::MakeObjGuard(listener, &::Spin::Listener::clearNewConnectionHandler);
	while (1)
	{
		boost::shared_ptr< ::Spin::Details::Request > request(request_handler.getNextRequest());
		// we handle only GET requests - anything else and we respond with 404
		if (request->method_ == "GET")
		{
			if (request->url_ == "/")
				request->connection_.write(
					"HTTP/1.1 302 Found\r\n"
					"Content-Length: 0\r\n"
					"Location: /index.html\r\n"
					"\r\n"
					);
			else if (request->url_ == "/index.html")
				request->connection_.write(
					"HTTP/1.1 200 OK\r\n"
					"Content-Length: 46\r\n"
					"Content-Type: text/html\r\n"
					"\r\n"
					"<html><body><p>Hello, world!</p></body></html>"
					);
		}
		else
		{
			request->connection_.write("HTTP/1.1 501 Not Implemented\r\n\r\n");
		}
	}
}
