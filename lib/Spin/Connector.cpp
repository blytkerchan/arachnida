#include "Connector.h"
extern "C" {
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
}
#include <boost/format.hpp>
#include <loki/ScopeGuard.h>

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
		::SSL_CTX * ssl_context = ::SSL_CTX_new(::SSLv23_client_method());
		if (!ssl_context)
		{
		        unsigned long error_code(ERR_get_error());
			int reason_code(ERR_GET_REASON(error_code));

		}
		else
		{ /* all is well */ }
		Loki::ScopeGuard ssl_context_guard = Loki::MakeGuard(SSL_CTX_free, ssl_context);
		/* We'd normally set some stuff like the verify paths and
		 * mode here because as things stand this will connect to
		 * any server whose certificate is signed by any CA.	*/
		::BIO * bio(BIO_new_ssl_connect(ssl_context));
		if (!bio)
			throw std::runtime_error("Failed to create connection socket"); // HERE too
		else
		{ /* all is well */ }
		Loki::ScopeGuard bio_guard = Loki::MakeGuard(::BIO_free, bio);
		SSL * ssl(0);
		BIO_get_ssl(bio, &ssl);
		if (!ssl)
			throw std::runtime_error("Can't locate SSL pointer");
		else
		{ /* all is well */ }
		SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
		// we might want to do some other things with the SSL HERE
		BIO_set_conn_hostname(bio, target.c_str());
		if (BIO_do_connect(bio) <= 0)
			throw std::runtime_error("Failed to create connection"); // HERE again
		else
		{ /* all is well */ }
		if (BIO_do_handshake(bio) <= 0)
			throw std::runtime_error("handshake failed"); // HERE again
		else
		{ /* all is well */ }

		bio_guard.Dismiss();
		ssl_context_guard.Dismiss();
		return bio;
	}

	::BIO * Connector::connect_(const std::string & target)
	{
		// BIO_new_connect wants a char*, not a const char*
		std::vector< char > buff(target.begin(), target.end());
		buff.push_back(0);
		::BIO * bio(::BIO_new_connect(&(buff[0])));
		if (!bio)
			throw std::runtime_error("Failed to create connection socket"); // HERE be more eloquent
		else
		{ /* all is well */ }
		Loki::ScopeGuard bio_guard = Loki::MakeGuard(::BIO_free, bio);
		if (::BIO_do_connect(bio) <= 0)
			throw std::runtime_error("Failed to create the connection");
		else
		{ /* all is well */ }
		bio_guard.Dismiss();
		return bio;
	}
}

