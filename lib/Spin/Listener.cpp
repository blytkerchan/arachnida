#include "Listener.h"
#include <cassert>
#include <boost/format.hpp>
#include "Connection.h"
extern "C" {
#include <openssl/bio.h>
#include <openssl/ssl.h>
}

namespace Spin
{
	Listener::Listener(Details::Address local_address, boost::uint16_t local_port)
		: bio_(0)
	{
		std::string local_address_s;
		if (local_address.u_.u32_ == 0)
			local_address_s = "*";
		else
		{
			boost::format fmt("%1%.%2%.%3%.%4%");
			fmt % local_address.u_.u8_[0]
				% local_address.u_.u8_[1]
				% local_address.u_.u8_[2]
				% local_address.u_.u8_[3]
				;
			local_address_s = fmt.str();
		}
		boost::format fmt("%1%:%2%");
		fmt % local_address_s;
		if (local_port == 0)
			fmt % "*";
		else
			fmt % local_port;
		bio_ = createBIO_(fmt.str());
		assert(bio_); // should have thrown otherwise
	}

	Listener::~Listener()
	{
		if (bio_)
			BIO_free(bio_);
		else
		{ /* no-op */ }
	}

	Connection Listener::accept()
	{
		/* Wait for incoming connection */
		if (BIO_do_accept(bio_) <= 0)
			throw std::runtime_error("Error accepting connection");
		else
		{ /* connection accepted OK */ }
		BIO * cbio(BIO_pop(bio_));
		assert(cbio);
		return Connection(cbio);
	}

	BIO * Listener::createBIO_(const std::string & local_address)
	{
		std::vector< char > b(local_address.begin(), local_address.end());
		b.push_back(0);
		BIO * bio(BIO_new_accept(&(b[0])));
		if (!bio)
			throw std::bad_alloc();
		else
		{ /* carry on */ }

		// First call to BIO_accept() sets up accept BIO
		if(BIO_do_accept(bio) <= 0)
			throw std::runtime_error("Error setting up accept");
		else
		{ /* all is well */ }
		return bio;
	}
}

