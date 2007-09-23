#include "Server.h"
#include <memory>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/Time_Value.h>
#include "Private/ScopedLock.h"
#include "Private/ObservationAdapter.h"
#include "Private/ConnectionHandler.h"
#include "Private/Logger.h"
#include "Handlers/NewConnectionHandler.h"

namespace Spin
{
	struct Server::ObservationHelper : Private::ObservationAdapter
	{
		ObservationHelper(Server * owner)
			: owner_(owner)
		{ /* no-op */ }
		~ObservationHelper()
		{ /* no-op */ }

		/*virtual */ACE_HANDLE getHandle() const/* = 0*/;
		/*virtual */void onReadReady();

		Server * owner_;
	};

	ACE_HANDLE Server::ObservationHelper::getHandle() const
	{
		return owner_->server_socket_->get_handle();
	}

	// this function is called when a connection is ready to be accepted
	void Server::ObservationHelper::onReadReady()
	{
		std::auto_ptr< ACE_SOCK_Stream > new_connection(new ACE_SOCK_Stream);
		ACE_INET_Addr remote_address;
		ACE_Time_Value timeout_for_poll(0, 0);	// as we're in a call-back, we should be pretty sure there's something here for us
		int rv(owner_->server_socket_->accept(*new_connection, &remote_address, &timeout_for_poll));
		if (rv == 0)
		{
			Private::ScopedLock lock(*(owner_->new_connection_handler_lock_));
			Handlers::NewConnectionHandler * handler(owner_->new_connection_handler_);
			if (handler)
				(*handler)(new_connection);
			else
			{ /* no-one to notify - nothing to do */
				Private::Logger::warning("Server", "No handler installed for new connections - new connection ignored");
			}
		}
		else
		{
			/* something went wrong - ignore this connection */
			Private::Logger::warning("Server", "Error while accepting a connection", strerror(errno));
		}
	}

	Server::Server(const std::string & address_to_bind, boost::uint16_t port_to_bind, Handlers::NewConnectionHandler * new_connection_handler/* = 0*/)
		: server_socket_(0),
		  observer_(0),
		  new_connection_handler_lock_(0),
		  new_connection_handler_(0)
	{
		std::auto_ptr< ACE_SOCK_Acceptor > server_socket(new ACE_SOCK_Acceptor(ACE_INET_Addr(port_to_bind, address_to_bind.c_str()), 0, PF_INET));
		/*
		 * ACE does not throw an exception if it can't open the socket, so we'll have to check the handle
		 */
		if (server_socket->get_handle() == ACE_INVALID_HANDLE)
			throw std::runtime_error("Failed to open socket");
		else
		{ /* all is well */ }
		std::auto_ptr< ObservationHelper > observer(new ObservationHelper(this));
		std::auto_ptr< ACE_Thread_Mutex > new_connection_handler_lock(new ACE_Thread_Mutex);
		observer_ = observer.get();
		server_socket_ = server_socket.get();
		new_connection_handler_lock_ = new_connection_handler_lock.get();
		new_connection_handler_ = new_connection_handler;
		Private::ConnectionHandler::getInstance().attach(observer.get());
		observer.release();
		server_socket.release();
		new_connection_handler_lock.release();
	}

	void Server::setNewConnectionHandler(Handlers::NewConnectionHandler * new_connection_handler)
	{
		if (new_connection_handler == 0 || (new_connection_handler_ == 0 && new_connection_handler != 0))
		{
			new_connection_handler_lock_->acquire();
			new_connection_handler_ = new_connection_handler;
			new_connection_handler_lock_->release();
		}
		else if (new_connection_handler_ != new_connection_handler)
			throw std::logic_error("Cannot set a new connection handler twice");
	}

	Server::~Server()
	{
		Private::ConnectionHandler::getInstance().detach(observer_);
		delete server_socket_;
	}
}
