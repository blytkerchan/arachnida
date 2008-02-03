#ifndef _spin_listener_h
#define _spin_listener_h

#include "Details/prologue.h"
#include <boost/cstdint.hpp>
#include <boost/scoped_ptr.hpp>
#include "Connection.h"
#include "Details/Address.h"
#include <Scorpion/Context.h>

#ifndef DOXYGEN_GENERATING
namespace boost { namespace filesystem { class path; } }
namespace Scorpion { class BIO; }
#endif
namespace Spin
{
	namespace Handlers
	{
		class NewConnectionHandler;
	}
	/** A basic server-implementing class. 
	 * The Listener class listens on any given local address and port and allows you to accept incoming 
	 * connections. If given a Scorpion context during construction, it will use the context to accept 
	 * SSL connections in stead of accepting unencrypted/unsecured connections. Once the context is in
	 * place, though, this is completely transparent.
	 *
	 * Connections can be accepted synchronously, using the accept method, or asynchronously by setting
	 * a handler for new connections, which is an instance of \link Handlers::NewConnectionHandler 
	 * NewConnectionHandler \endlink. 
	 *
	 * When using SSL, it is important to note that the security is only as good as the context you set
	 * up for it. For example: if you want your server to check the validity of its clients, you need to
	 * set the context up to do those checks before constructing the Listener as you will not be able to 
	 * do it afterwards. */
	class SPIN_API Listener
	{
	public :
		//! Construct a non-secured server
		Listener(Details::Address local_address, boost::uint16_t local_port);
		//! Construct a secured server
		Listener(const Scorpion::Context & security_context, Details::Address local_address, boost::uint16_t local_port);
		~Listener();

		//! Synchronously accept a connection (i.e. wait for it)
		boost::shared_ptr< Connection > accept();

		//! Set a new connection handler for accepting connections asynchronously
		void setNewConnectionHandler(Handlers::NewConnectionHandler & handler);
		//! Clear the handler for asynchronous new connections
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
