#include "BIO.h"
#include <vector>
extern "C" {
#include <openssl/ssl.h>
}
#include <loki/ScopeGuard.h>
#include "Context.h"
#include "Private/OpenSSL.h"
#include "Exceptions/SSL.h"

namespace Scorpion
{
	class ConnectionBIO : public BIO
	{
	public :
		using BIO::setBIO;

		ConnectionBIO()
		{ /* no-op */ }

		/*virtual */void reset()/* = 0*/
		{
			BIO_reset(getBIO());
		}

		/*virtual */int read(void * buffer, std::size_t buffer_size)
		{
			return ::BIO_read(getBIO(), buffer, static_cast< int >(buffer_size));
		}

		/*virtual */int write(const void * buffer, std::size_t buffer_size)
		{
			return ::BIO_write(getBIO(), buffer, static_cast< int >(buffer_size));
		}

		/*virtual */bool poll() const
		{
			return BIO_pending(getBIO()) != 0;
		}

		/*virtual */bool shouldRetry() const { return BIO_should_retry(getBIO()) != 0; }
		/*virtual */bool shouldRead() const { return BIO_should_read(getBIO()) != 0; }
		/*virtual */bool shouldWrite() const { return BIO_should_write(getBIO()) != 0; }
	};

	class AcceptBio : public BIO
	{
	public :
		using BIO::setBIO;

		AcceptBio()
		{ /* no-op */ }

		/*virtual */void reset()/* = 0*/
		{
			BIO_reset(getBIO());
		}

		/*virtual */BIO * accept() const
		{
			std::auto_ptr< ConnectionBIO > retval(new ConnectionBIO);

			/* Wait for incoming connection */
			SCORPION_PRIVATE_OPENSSL_EXEC(, ::BIO_do_accept(getBIO()) > 0, Exceptions::SSL::AcceptError);
			::BIO * cbio(::BIO_pop(getBIO()));
			assert(cbio);
			retval->setBIO(cbio);
			SSL * ssl(0);
			BIO_get_ssl(cbio, &ssl);
			if (ssl)
			{
				SCORPION_PRIVATE_OPENSSL_EXEC(, BIO_do_handshake(cbio) > 0, Exceptions::SSL::HandshakeError);
			}
			else
			{ /* not an SSL BIO - no handshake to do */ }
			return retval.release();
		}
	};

	/*virtual */BIO::~BIO()
	{ /* no-op */ }

	bool BIO::usesSSL() const
	{
		SSL * ssl(0);
		BIO_get_ssl(getBIO(), &ssl);
		return ssl != 0;
	}

	int BIO::getFD() const
	{
		return ::BIO_get_fd(openssl_bio_.get(), 0);
	}

	BIO::BIO()
	{ /* no-op */ }

	void BIO::setBIO(::BIO * openssl_bio)
	{
		openssl_bio_.reset(openssl_bio, freeOpenSSLBIO);
	}

	/*static */void BIO::freeOpenSSLBIO(::BIO * bio)
	{
		::BIO_free_all(bio);
	}

	BIO * createAcceptBIO(const std::string & local_address, int flags)
	{
		std::vector< char > b(local_address.begin(), local_address.end());
		b.push_back(0);
		SCORPION_PRIVATE_OPENSSL_EXEC(::BIO * bio(BIO_new_accept(&(b[0]))), bio, Exceptions::SSL::AcceptSocketAllocationError);
		Loki::ScopeGuard accept_bio_guard = Loki::MakeGuard(::BIO_free_all, bio);
		// First call to BIO_accept() sets up accept BIO
		SCORPION_PRIVATE_OPENSSL_EXEC(, BIO_do_accept(bio) > 0, Exceptions::SSL::AcceptSetupError);
		std::auto_ptr< AcceptBio > retval(new AcceptBio);
		accept_bio_guard.Dismiss();
		retval->setBIO(bio);

		return retval.release();
	}

	BIO * createSSLAcceptBIO(const std::string & local_address, Context & security_context, int flags)
	{
		::SSL_CTX * ssl_context(security_context.getContext());

		// New SSL BIO setup as server
		SCORPION_PRIVATE_OPENSSL_EXEC(::BIO * ssl_bio(::BIO_new_ssl(ssl_context, 0)), ssl_bio, Exceptions::SSL::ServerSocketAllocationError);
		Loki::ScopeGuard ssl_bio_guard = Loki::MakeGuard(::BIO_free_all, ssl_bio);

		SCORPION_PRIVATE_OPENSSL_EXEC(SSL * ssl(0); ::BIO_get_ssl(ssl_bio, &ssl), ssl, Exceptions::SSL::SSLPointerLocationError);

		std::vector< char > b(local_address.begin(), local_address.end());
		b.push_back(0);
		SCORPION_PRIVATE_OPENSSL_EXEC(::BIO * accept_bio(::BIO_new_accept(&(b[0]))), accept_bio, Exceptions::SSL::AcceptSocketAllocationError);
		Loki::ScopeGuard accept_bio_guard = Loki::MakeGuard(::BIO_free_all, accept_bio);

		/*
		 * By doing this when a new connection is established
		 * we automatically have the ssl_bio inserted into it. The
		 * BIO chain is now 'swallowed' by the accept BIO and
		 * will be freed when the accept BIO is freed.
		 */
		BIO_set_accept_bios(accept_bio, ssl_bio);
		ssl_bio_guard.Dismiss();

		// Setup accept BIO
		SCORPION_PRIVATE_OPENSSL_EXEC(, BIO_do_accept(accept_bio) > 0, Exceptions::SSL::AcceptSetupError);
		std::auto_ptr< AcceptBio > retval(new AcceptBio);
		accept_bio_guard.Dismiss();
		retval->setBIO(accept_bio);

		return retval.release();
	}

	BIO * connect(const Context & context, const std::string & peer_address)
	{
		std::auto_ptr< ConnectionBIO > retval(new ConnectionBIO);

		::SSL_CTX * ssl_context(context.getContext());
		SCORPION_PRIVATE_OPENSSL_EXEC(::BIO * bio(BIO_new_ssl_connect(ssl_context)), bio, Exceptions::SSL::ConnectionSocketCreationFailure);
		retval->setBIO(bio);
		SCORPION_PRIVATE_OPENSSL_EXEC(SSL * ssl(0); BIO_get_ssl(bio, &ssl), ssl, Exceptions::SSL::SSLPointerLocationError);

		// we might want to do some other things with the SSL HERE
		BIO_set_conn_hostname(bio, peer_address.c_str());
		SCORPION_PRIVATE_OPENSSL_EXEC(, BIO_do_connect(bio) > 0, Exceptions::SSL::ConnectionError);
		SCORPION_PRIVATE_OPENSSL_EXEC(, BIO_do_handshake(bio) > 0, Exceptions::SSL::HandshakeError);

		return retval.release();
	}

	BIO * connect(const std::string & peer_address)
	{
		std::auto_ptr< ConnectionBIO > retval(new ConnectionBIO);

		// BIO_new_connect wants a char*, not a const char*
		std::vector< char > buff(peer_address.begin(), peer_address.end());
		buff.push_back(0);
		SCORPION_PRIVATE_OPENSSL_EXEC(::BIO * bio(::BIO_new_connect(&(buff[0]))), bio, Exceptions::SSL::ConnectionSocketCreationFailure);
		retval->setBIO(bio);
		SCORPION_PRIVATE_OPENSSL_EXEC(, BIO_do_connect(bio) > 0, Exceptions::SSL::ConnectionError);
		return retval.release();
	}
}

