#include "HTTPDataHandler.h"
#include "../Connection.h"
#include "../Details/Request.h"
#include "../Private/RequestParser.h"

namespace Spin
{
	namespace Handlers
	{
		/*virtual */void HTTPDataHandler::onDataReady(Connection & connection) const throw()/* = 0*/
		{
			/* For the time being, we will try to obtain the entire request 
			 * here. Obtaining the request is a blocking operation so it's 
			 * really not a good idea to do that here completely. It would 
			 * be nice if we could associate some data with the connection, 
			 * or perhaps with all connections, that would allow us to do 
			 * this is a non-blocking manner (remember we are being called 
			 * in the context of the connection handler's worker thread here, 
			 * so anything blocking we do here will block all connections).
			 * This is therefore a definite FIXME: obtaining the request should
			 * neither be mandatory (malicious and buggy clients may just send
			 * us some kind of bullshit, keep the connection open until we slam
			 * the door, etc.) nor blocking (in the name of latency, nothing 
			 * should be blocking in this context. */
			std::vector< char > buffer;
			std::pair< std::size_t, int > rv(connection.read(buffer));
			std::string str(buffer.begin(), buffer.end());
			if (str.find("\n\n") != std::string::npos ||
				str.find("\r\r") != std::string::npos ||
				str.find("\n\r\n\r") != std::string::npos)
			{	// end of the headers (if any) found
				Details::Request request(Private::parse(str));
			}
			else
			{ /* ah, to hell with it! */ }
		}
	}
}

