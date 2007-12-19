#ifndef _spin_listener_h
#define _spin_listener_h

#include "Details/prologue.h"
#include <boost/cstdint.hpp>
#include <boost/scoped_ptr.hpp>
#include "Connection.h"
#include "Details/Address.h"
#include <Scorpion/Context.h>

namespace boost { namespace filesystem { class path; } }
namespace Scorpion { class BIO; }
namespace Spin
{
	namespace Handlers
	{
		class NewConnectionHandler;
	}
	class SPIN_API Listener
	{
	public :
		Listener(Details::Address local_address, boost::uint16_t local_port);
		Listener(const Scorpion::Context & security_context, Details::Address local_address, boost::uint16_t local_port);
		~Listener();

		Connection accept();

		void setNewConnectionHandler(Handlers::NewConnectionHandler & handler);
		void clearNewConnectionHandler();

	private :
		// Neither CopyConstructible nor Assignable
		Listener(const Listener&);
		Listener & operator=(const Listener&);

		std::string constructLocalAddress_(Details::Address local_address, boost::uint16_t local_port);
		Scorpion::BIO * createBIO_(const std::string & local_address);
		Scorpion::BIO * createSSLBIO_(const std::string & local_address);

		void onNewConnection_();

		Scorpion::Context security_context_;
		boost::scoped_ptr< Scorpion::BIO > bio_;
		Handlers::NewConnectionHandler * new_connection_handler_;
	};
}

#endif
