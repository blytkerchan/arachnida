#include "HTTPConnectionHandler.h"
#include <list>
#include <boost/bind.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include "HTTPDataHandler.h"
#include "../Connection.h"
#include "../Details/Address.h"

namespace Spin
{
	namespace Handlers
	{
		namespace
		{
			template < typename T >
			struct Is_
			{
				Is_(T t) : t_(t) {}

				bool operator()(T t) { return t_ == t; }
				template < typename Y > bool operator()(Y y) { return y.get() == t_; }

				T t_;
			};

			template < typename T >
			Is_<T> is(T t)
			{
				return Is_< T >(t);
			}

			template < typename Container >
			struct ErrorHandler
			{
				ErrorHandler(Container & container)
					: container_(container)
				{}

				void operator()(Connection * connection) const
				{
					Container::iterator where(
						std::find_if(
							container_.begin(),
							container_.end(),
							is(connection)
							)
						);
					if (where != container_.end())
						container_.erase(where);
					else
					{ /* not found */ }
				}

				Container & container_;
			};
		}

		struct HTTPConnectionHandler::Data
		{
			typedef std::list< boost::shared_ptr< Connection > > Connections_;

			Data(HTTPRequestHandler & request_handler)
				: data_handler_(request_handler)
			{ /* no-op */ }

			/* WARNING: declaration order is important here: the connections 
			 * must be destroyed before the data handler is. The reason 
			 * for this is simple: the connections use the data handler 
			 * and won't know when it's been destroyed (we're not using
			 * weak pointers here) so we must guarantee that they stay alive
			 * at least as long as the connections do. Declaration order and
			 * language semantics does that for us. */
			HTTPDataHandler data_handler_;
			Connections_ connections_;
		};

		HTTPConnectionHandler::HTTPConnectionHandler(HTTPRequestHandler & request_handler)
			: data_(new Data(request_handler)),
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
			boost::shared_ptr< Connection > connection_p(new Connection(connection));
			connection_p->setNewDataHandler(data_->data_handler_, boost::bind<void>(ErrorHandler< Data::Connections_ >(data_->connections_), connection_p.get()));
			data_->connections_.push_back(connection_p);
		}

		/*virtual */bool HTTPConnectionHandler::validate(const Details::Address & peer_address)
		{
			return true;
		}
	}
}

