#include "Connection.h"
extern "C" {
#include <openssl/bio.h>
}

namespace Spin
{
	Connection::Connection(const Connection & connection)
		: bio_(connection.bio_)
	{
		connection.bio_ = 0;
	}

	Connection::~Connection()
	{
		if (bio_)
			::BIO_free_all(bio_);
		else
		{ /* nothing to do here */ }
	}

	void Connection::write(const std::vector< char > & data)
	{
	}

	void Connection::read(std::vector< char > & buffer)
	{
	}

	std::string Connection::getLocalAddress() const
	{
		return std::string();
	}

	boost::uint16_t Connection::getLocalPort() const
	{
		return 0;
	}

	std::string Connection::getRemoteAddress() const
	{
		return std::string();
	}

	boost::uint16_t Connection::getRemotePort() const
	{
		return 0;
	}

	bool Connection::usesSSL() const
	{
		return false;
	}

	Connection::Connection(::BIO * bio)
		: bio_(bio)
	{ /* no-op */ }
}

