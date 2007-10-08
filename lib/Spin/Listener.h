#ifndef _spin_listener_h
#define _spin_listener_h

#include "Details/prologue.h"
#include <boost/cstdint.hpp>
#include "Connection.h"
#include "Details/Address.h"

typedef struct bio_st BIO;
namespace boost { namespace filesystem { class path; } }
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
		Listener(const boost::filesystem::path & server_cert_filename, Details::Address local_address, boost::uint16_t local_port);
		~Listener();

		Connection accept();

		void setNewConnectionHandler(Handlers::NewConnectionHandler & handler);
		void clearNewConnectionHandler();

	private :
		// Neither CopyConstructible nor Assignable
		Listener(const Listener&);
		Listener & operator=(const Listener&);

		std::string constructLocalAddress_(Details::Address local_address, boost::uint16_t local_port);
		BIO * createBIO_(const std::string & local_address);
		BIO * createSSLBIO_(const boost::filesystem::path & server_cert_filename, const std::string & local_address);

		void onNewConnection_();

		BIO * bio_;
		Handlers::NewConnectionHandler * new_connection_handler_;
	};
}

#endif
