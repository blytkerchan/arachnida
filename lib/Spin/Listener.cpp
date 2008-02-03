#include "Listener.h"
#include <cassert>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <loki/ScopeGuard.h>
#include <Scorpion/BIO.h>
extern "C" {
#include <openssl/bio.h>
#include <openssl/ssl.h>
}
#include "Connection.h"
#include "Private/ConnectionHandler.h"
#include "Handlers/NewConnectionHandler.h"

namespace Spin
{
	Listener::Listener(Details::Address local_address, boost::uint16_t local_port)
		: bio_(createBIO_(constructLocalAddress_(local_address, local_port))),
		  new_connection_handler_(0)
	{ /* no-op */ }

	Listener::Listener(const Scorpion::Context & security_context, Details::Address local_address, boost::uint16_t local_port)
		: security_context_(security_context),
		  bio_(createSSLBIO_(constructLocalAddress_(local_address, local_port))),
		  new_connection_handler_(0)
	{ /* no-op */ }

	Listener::~Listener()
	{
		clearNewConnectionHandler();
	}

	boost::shared_ptr< Connection > Listener::accept()
	{
		return boost::shared_ptr< Connection >(new Connection(bio_->accept()));
	}

	void Listener::setNewConnectionHandler(Handlers::NewConnectionHandler & handler)
	{
		new_connection_handler_ = &handler;
		Private::ConnectionHandler::getInstance().attach(bio_->getFD(), boost::bind(&Listener::onNewConnection_, this));
	}

	void Listener::clearNewConnectionHandler()
	{
		if (new_connection_handler_)
		{
			Private::ConnectionHandler::getInstance().detach(bio_->getFD());
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

	Scorpion::BIO * Listener::createBIO_(const std::string & local_address)
	{
		return Scorpion::createAcceptBIO(local_address, Scorpion::BIO::non_blocking__);
	}

	Scorpion::BIO * Listener::createSSLBIO_(const std::string & local_address)
	{
		return Scorpion::createSSLAcceptBIO(local_address, security_context_, Scorpion::BIO::auto_retry__ | Scorpion::BIO::non_blocking__);
	}

	void Listener::onNewConnection_()
	{
		if (new_connection_handler_)
			(*new_connection_handler_)(accept());
		else
		{ /* no new connection handler set */ }
	}
}

