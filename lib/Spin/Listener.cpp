#include "Listener.h"
#include <cassert>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <loki/ScopeGuard.h>
extern "C" {
#include <openssl/bio.h>
#include <openssl/ssl.h>
}
#include "Connection.h"
#include "Private/ConnectionHandler.h"
#include "Private/OpenSSL.h"
#include "Handlers/NewConnectionHandler.h"
#include "Exceptions/SSL.h"

namespace Spin
{
	Listener::Listener(Details::Address local_address, boost::uint16_t local_port)
		: bio_(0),
		  new_connection_handler_(0)
	{
		bio_ = createBIO_(constructLocalAddress_(local_address, local_port));
		assert(bio_); // should have thrown otherwise
	}

	Listener::Listener(const boost::filesystem::path & server_cert_filename, Details::Address local_address, boost::uint16_t local_port)
		: bio_(0),
		  new_connection_handler_(0)
	{
		bio_ = createSSLBIO_(server_cert_filename, constructLocalAddress_(local_address, local_port));
		assert(bio_); // should have thrown otherwise
	}

	Listener::~Listener()
	{
		clearNewConnectionHandler();
		if (bio_)
			BIO_free(bio_);
		else
		{ /* no-op */ }
	}

	Connection Listener::accept()
	{
		/* Wait for incoming connection */
		SPIN_PRIVATE_OPENSSL_EXEC(, BIO_do_accept(bio_) > 0, Exceptions::SSL::AcceptError);
		BIO * cbio(BIO_pop(bio_));
		assert(cbio);
		SSL * ssl(0);
		BIO_get_ssl(cbio, &ssl);
		if (ssl)
		{
			SPIN_PRIVATE_OPENSSL_EXEC(, BIO_do_handshake(cbio) > 0, Exceptions::SSL::HandshakeError);
		}
		else
		{ /* not an SSL BIO - no handshake to do */ }
		return Connection(cbio);
	}

	void Listener::setNewConnectionHandler(Handlers::NewConnectionHandler & handler)
	{
		new_connection_handler_ = &handler;
		Private::ConnectionHandler::getInstance().attach(BIO_get_fd(bio_, 0), boost::bind(&Listener::onNewConnection_, this));
	}

	void Listener::clearNewConnectionHandler()
	{
		if (new_connection_handler_)
		{
			Private::ConnectionHandler::getInstance().detach(BIO_get_fd(bio_, 0));
			new_connection_handler_ = 0;
		}
		else
		{ /* nothing to clear */ }
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
		SPIN_PRIVATE_OPENSSL_EXEC(BIO * bio(BIO_new_accept(&(b[0]))), bio, Exceptions::SSL::AcceptSocketAllocationError);
		// First call to BIO_accept() sets up accept BIO
		SPIN_PRIVATE_OPENSSL_EXEC(, BIO_do_accept(bio) > 0, Exceptions::SSL::AcceptSetupError);
		return bio;
	}

	BIO * Listener::createSSLBIO_(const boost::filesystem::path & server_cert_filename, const std::string & local_address)
	{
		SPIN_PRIVATE_OPENSSL_EXEC(SSL_CTX * ssl_context(SSL_CTX_new(SSLv23_server_method())), ssl_context, Exceptions::SSL::SSLContextAllocationError);
		Loki::ScopeGuard ssl_context_guard = Loki::MakeGuard(SSL_CTX_free, ssl_context);

		SPIN_PRIVATE_OPENSSL_EXEC(, SSL_CTX_use_certificate_file(ssl_context, server_cert_filename.string().c_str(), SSL_FILETYPE_PEM) && SSL_CTX_use_PrivateKey_file(ssl_context, server_cert_filename.string().c_str(), SSL_FILETYPE_PEM) && SSL_CTX_check_private_key(ssl_context), Exceptions::SSL::SSLContextSetupError);

		/* HERE!!
		 * Might do other things here like setting verify locations and
		 * DH and/or RSA temporary key callbacks		*/

		// New SSL BIO setup as server
		SPIN_PRIVATE_OPENSSL_EXEC(BIO * ssl_bio(BIO_new_ssl(ssl_context, 0)), ssl_bio, Exceptions::SSL::ServerSocketAllocationError);
		Loki::ScopeGuard ssl_bio_guard = Loki::MakeGuard(BIO_free, ssl_bio);

		SPIN_PRIVATE_OPENSSL_EXEC(SSL * ssl(0); BIO_get_ssl(ssl_bio, &ssl), ssl, Exceptions::SSL::SSLPointerLocationError);

		// Don't want any retries
		SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

		std::vector< char > b(local_address.begin(), local_address.end());
		b.push_back(0);
		SPIN_PRIVATE_OPENSSL_EXEC(BIO * accept_bio(BIO_new_accept(&(b[0]))), accept_bio, Exceptions::SSL::AcceptSocketAllocationError);

		/*
		 * By doing this when a new connection is established
		 * we automatically have the ssl_bio inserted into it. The
		 * BIO chain is now 'swallowed' by the accept BIO and
		 * will be freed when the accept BIO is freed.
		 */

		BIO_set_accept_bios(accept_bio, ssl_bio);
		ssl_bio_guard.Dismiss();

		// Setup accept BIO
		SPIN_PRIVATE_OPENSSL_EXEC(, BIO_do_accept(accept_bio) > 0, Exceptions::SSL::AcceptSetupError);
		ssl_context_guard.Dismiss();
		return accept_bio;
	}

	void Listener::onNewConnection_()
	{
		if (new_connection_handler_)
			(*new_connection_handler_)(accept());
		else
		{ /* no new connection handler set */ }
	}
}

