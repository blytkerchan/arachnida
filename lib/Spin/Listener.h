#ifndef _spin_listener_h
#define _spin_listener_h

#include "Details/prologue.h"
#include <boost/cstdint.hpp>
#include "Connection.h"
#include "Details/Address.h"

typedef struct bio_st BIO;
namespace Spin
{
	class SPIN_API Listener
	{
	public :
		Listener(Details::Address local_address, boost::uint16_t local_port);
		~Listener();

		Connection accept();

	private :
		// Neither CopyConstructible nor Assignable
		Listener(const Listener&);
		Listener & operator=(const Listener&);

		BIO * createBIO_(const std::string & local_address);

		// BIO * createSSLBIO_(const std::string & local_address,  const std::string & server_certificate_file); // see BIO_f_ssl(3)

		BIO * bio_;
	};
}

#endif
