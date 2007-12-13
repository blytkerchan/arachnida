#include "Connector.h"
extern "C" {
#include <openssl/bio.h>
#include <openssl/ssl.h>
}
#include <boost/format.hpp>
#include <loki/ScopeGuard.h>
#include "Exceptions/SSL.h"
#include "Private/OpenSSL.h"

namespace Spin
{
	/*static */Connector & Connector::getInstance()
	{
		static Connector instance__;
		return instance__;
	}

	Connection Connector::connect(const std::string & remote_address, boost::uint16_t port, bool use_ssl/* = false*/)
	{
		boost::format fmt("%1%:%2%");
		fmt % remote_address % port;
		::BIO * bio(use_ssl ? connectSSL_(fmt.str()) : connect_(fmt.str()));
		return Connection(bio);
	}

	Connector::Connector()
	{ /* no-op */ }

	Connector::~Connector()
	{ /* no-op */ }

	::BIO * Connector::connectSSL_(const std::string & target)
	{
		/* Note that contrary to common practice, we create an SSL context object for
		 * each connection, rather than a single context object that is shared by all
		 * connections. We do this to avoid certain common pitfalls with connections
		 * and SSL contexts.
		 * At some later time, we might want to make this (compile-time) configurable */
		SPIN_PRIVATE_OPENSSL_EXEC(::SSL_CTX * ssl_context(::SSL_CTX_new(::SSLv23_client_method())), ssl_context, Exceptions::SSL::ContextAllocationError);
		Loki::ScopeGuard ssl_context_guard = Loki::MakeGuard(SSL_CTX_free, ssl_context);
		/* We'd normally set some stuff like the verify paths and
		 * mode here because as things stand this will connect to
		 * any server whose certificate is signed by any CA.	*/
		SPIN_PRIVATE_OPENSSL_EXEC(::BIO * bio(BIO_new_ssl_connect(ssl_context)), bio, Exceptions::SSL::ConnectionSocketCreationFailure);
		Loki::ScopeGuard bio_guard = Loki::MakeGuard(::BIO_free, bio);
		SPIN_PRIVATE_OPENSSL_EXEC(SSL * ssl(0); BIO_get_ssl(bio, &ssl), ssl, Exceptions::SSL::SSLPointerLocationError);
		SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
		// we might want to do some other things with the SSL HERE
		BIO_set_conn_hostname(bio, target.c_str());
		SPIN_PRIVATE_OPENSSL_EXEC(, BIO_do_connect(bio) > 0, Exceptions::SSL::ConnectionError);
		SPIN_PRIVATE_OPENSSL_EXEC(, BIO_do_handshake(bio) > 0, Exceptions::SSL::HandshakeError);

		bio_guard.Dismiss();
		ssl_context_guard.Dismiss();
		return bio;
	}

	::BIO * Connector::connect_(const std::string & target)
	{
		// BIO_new_connect wants a char*, not a const char*
		std::vector< char > buff(target.begin(), target.end());
		buff.push_back(0);
		SPIN_PRIVATE_OPENSSL_EXEC(::BIO * bio(::BIO_new_connect(&(buff[0]))), bio, Exceptions::SSL::ConnectionSocketCreationFailure);
		Loki::ScopeGuard bio_guard = Loki::MakeGuard(::BIO_free, bio);
		SPIN_PRIVATE_OPENSSL_EXEC(, BIO_do_connect(bio) > 0, Exceptions::SSL::ConnectionError);
		bio_guard.Dismiss();
		return bio;
	}
}

