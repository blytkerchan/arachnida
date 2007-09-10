#ifndef _spin_server_h
#define _spin_server_h

#include <boost/cstdint.hpp>

class ACE_SOCK_Acceptor;
class ACE_Thread_Mutex;
namespace Spin
{
	namespace Handlers
	{
		class NewConnectionHandler;
	}
	class Server
	{
	public :
		Server(boost::uint32_t address_to_bind, boost::uint16_t port_to_bind, Handlers::NewConnectionHandler * new_connection_handler = 0);
		~Server();

		void setNewConnectionHandler(Handlers::NewConnectionHandler * new_connection_handler);

	private :
		// Neither CopyConstructible nor Assignable
		Server(const Server &);
		Server & operator=(const Server &);

		struct ObservationHelper;

		ACE_SOCK_Acceptor * server_socket_;
		ObservationHelper * observer_;
		ACE_Thread_Mutex * new_connection_handler_lock_;
		Handlers::NewConnectionHandler * new_connection_handler_;

		friend struct ObservationHelper;
	};
}

#endif
