#include "HTTPConnectionHandler.h"
#include <boost/ptr_container/ptr_list.hpp>
#include "HTTPDataHandler.h"
#include "../Connection.h"
#include "../Details/Address.h"

namespace Spin
{
	namespace Handlers
	{
		struct HTTPConnectionHandler::Data
		{
			boost::ptr_list< Connection > connections_;
			std::list< HTTPDataHandler > data_handlers_;
		};

		HTTPConnectionHandler::HTTPConnectionHandler(HTTPRequestHandler & request_handler)
			: data_(new Data),
			  request_handler_(request_handler)
		{ /* no-op */ }

		HTTPConnectionHandler::~HTTPConnectionHandler()
		{
			delete data_;
		}

		/*virtual */void HTTPConnectionHandler::handleNewConnection(const Connection & connection)/* = 0*/
		{
			/* When we get called, we know that:
			 * * a new connection is ready and, if it is an SSL connection, 
			 *   the handshake has already taken place, meaning we can 
			 *   continue using the connection regardless of whether it's 
			 *   an SSL connection or not (that is transparent at this 
			 *   point).
			 * * the comm/messaging protocol used on this connection is 
			 *   HTTP, which means it will be using the HTTPDataHandler
			 * * we are still in the context of the ConnectionHandler's
			 *   worker thread, so anything we do here will add to the 
			 *   latency of the server
			 * * this is where the choice whether or not we should serve 
			 *   this client should take place.
			 */
			Details::Address peer_address(connection.getPeerAddress());
			if (!validate(peer_address))
			{
				return; // validate returned false - don't serve this client
			}
			else
			{ /* continue */ }
			/* Now that we've validated that we should, in deed, handle 
			 * this connection, we will copy the connection into its final
			 * object, after which we will attach a data handler to it, to
			 * which we will pass our request handler. 
			 * Note that the data handler can handle only one connection, 
			 * because the connection handler won't tell it which connection
			 * is ready to receive data. */
			std::auto_ptr< Connection > connection_p(new Connection(connection));
			{
				HTTPDataHandler data_handler(request_handler_);
				data_->data_handlers_.push_back(data_handler);
			}
			connection_p->setNewDataHandler(data_->data_handlers_.back());
			data_->connections_.push_back(connection_p.release());
		}

		/*virtual */bool HTTPConnectionHandler::validate(const Details::Address & peer_address)
		{
			return true;
		}
	}
}

