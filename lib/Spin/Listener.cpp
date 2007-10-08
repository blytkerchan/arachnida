#include "Listener.h"
#include <cassert>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <loki/ScopeGuard.h>
extern "C" {
#include <openssl/bio.h>
#include <openssl/ssl.h>
}
#include "Connection.h"

namespace Spin
{
	Listener::Listener(Details::Address local_address, boost::uint16_t local_port)
		: bio_(0)
	{
		bio_ = createBIO_(constructLocalAddress_(local_address, local_port));
		assert(bio_); // should have thrown otherwise
	}

	Listener::Listener(const boost::filesystem::path & server_cert_filename, Details::Address local_address, boost::uint16_t local_port)
	{
		bio_ = createSSLBIO_(server_cert_filename, constructLocalAddress_(local_address, local_port));
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

	std::string Listener::constructLocalAddress_(Details::Address local_address, boost::uint16_t local_port)
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

		return fmt.str();
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

	BIO * Listener::createSSLBIO_(const boost::filesystem::path & server_cert_filename, const std::string & local_address)
	{
		SSL_CTX * ssl_context(SSL_CTX_new(SSLv23_server_method()));
		if (!ssl_context)
			throw std::bad_alloc();
		else
		{ /* all is well */ }
		Loki::ScopeGuard ssl_context_guard = Loki::MakeGuard(SSL_CTX_free, ssl_context);

		if (!SSL_CTX_use_certificate_file(ssl_context, server_cert_filename.string().c_str(), SSL_FILETYPE_PEM) ||
			!SSL_CTX_use_PrivateKey_file(ssl_context, server_cert_filename.string().c_str(), SSL_FILETYPE_PEM) ||
			!SSL_CTX_check_private_key(ssl_context))
			throw std::runtime_error("Error setting up SSL_CTX");
		else
		{ /* all is well so far */ }

		/* HERE!!
		 * Might do other things here like setting verify locations and
		 * DH and/or RSA temporary key callbacks		*/

		// New SSL BIO setup as server
		BIO * ssl_bio(BIO_new_ssl(ssl_context, 0));
		if (!ssl_bio)
			throw std::bad_alloc();
		else
		{ /* all is well */ }
		Loki::ScopeGuard ssl_bio_guard = Loki::MakeGuard(BIO_free, ssl_bio);

		SSL * ssl(0);
		BIO_get_ssl(ssl_bio, &ssl);
		if (!ssl)
			throw std::runtime_error("Can't locate SSL pointer");
		else
		{ /* all is well */ }

		// Don't want any retries
		SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

		std::vector< char > b(local_address.begin(), local_address.end());
		b.push_back(0);
		BIO * accept_bio(BIO_new_accept(&(b[0])));
		if (!accept_bio)
			throw std::bad_alloc();
		else
		{ /* all is well */ }

		/*
		 * By doing this when a new connection is established
		 * we automatically have the ssl_bio inserted into it. The
		 * BIO chain is now 'swallowed' by the accept BIO and
		 * will be freed when the accept BIO is freed.
		 */

		BIO_set_accept_bios(accept_bio, ssl_bio);
		ssl_bio_guard.Dismiss();

		// Setup accept BIO
		if (BIO_do_accept(accept_bio) <= 0)
			throw std::runtime_error("Error setting up accept BIO");
		else
		{ /* all is well */ }
		ssl_context_guard.Dismiss();
		return accept_bio;
	}
}

