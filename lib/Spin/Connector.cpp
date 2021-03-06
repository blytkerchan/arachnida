#include "Connector.h"
extern "C" {
#include <openssl/bio.h>
#include <openssl/ssl.h>
}
#include <boost/format.hpp>
#include <loki/ScopeGuard.h>
#include <Scorpion/BIO.h>

namespace Spin
{
	/*static */Connector & Connector::getInstance()
	{
		static Connector instance__;
		return instance__;
	}

	boost::shared_ptr< Connection > Connector::connect(const Details::Address & remote_address, boost::uint16_t port)
	{
		boost::format fmt("%1%.%2%.%3%.%4%:%5%");
		fmt % (unsigned int)remote_address[0] % (unsigned int)remote_address[1] % (unsigned int)remote_address[2] % (unsigned int)remote_address[3] % port;
		Scorpion::BIO * bio(connect_(fmt.str()));
		return boost::shared_ptr< Connection > (new Connection(bio));
	}

	boost::shared_ptr< Connection > Connector::connect(const std::string & remote_address, boost::uint16_t port)
	{
		boost::format fmt("%1%:%2%");
		fmt % remote_address % port;
		Scorpion::BIO * bio(connect_(fmt.str()));
		return boost::shared_ptr< Connection > (new Connection(bio));
	}

	boost::shared_ptr< Connection > Connector::connect(const Scorpion::Context & context, const Details::Address & remote_address, boost::uint16_t port)
	{
		boost::format fmt("%1%.%2%.%3%.%4%:%5%");
		fmt % remote_address[0] % remote_address[1] % remote_address[2] % remote_address[3] % port;
		Scorpion::BIO * bio(connectSSL_(context, fmt.str()));
		return boost::shared_ptr< Connection > (new Connection(bio));
	}

	boost::shared_ptr< Connection > Connector::connect(const Scorpion::Context & context, const std::string & remote_address, boost::uint16_t port)
	{
		boost::format fmt("%1%:%2%");
		fmt % remote_address % port;
		Scorpion::BIO * bio(connectSSL_(context, fmt.str()));
		return boost::shared_ptr< Connection > (new Connection(bio));
	}

	Connector::Connector()
	{ /* no-op */ }

	Connector::~Connector()
	{ /* no-op */ }

	Scorpion::BIO * Connector::connectSSL_(const Scorpion::Context & context, const std::string & target)
	{
		return Scorpion::connect(context, target);
	}

	Scorpion::BIO * Connector::connect_(const std::string & target)
	{
		return Scorpion::connect(target);
	}
}

