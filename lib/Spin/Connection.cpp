#include "Connection.h"
extern "C" {
#include <openssl/bio.h>
#include <openssl/ssl.h>
}
#include <cassert>
#include <climits>
#include <Windows.h>
#include <boost/bind.hpp>
#include "Private/ConnectionHandler.h"
#include "Handlers/NewDataHandler.h"

namespace Spin
{
	Connection::Connection(const Connection & connection)
		: bio_(connection.bio_),
		  data_handler_(0)
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

	std::pair< std::size_t, int > Connection::write(const std::vector< char > & data)
	{
		assert(data.size() < INT_MAX);
		int written(0);
		int reason(no_error__);

		if (!data.empty())
		{
			written = ::BIO_write(bio_, &(data[0]), data.size());
			if (written == -2)
				throw std::logic_error("Method not implemented on this BIO");
			else if (written < static_cast< int >(data.size()))
			{
				if (!BIO_should_retry(bio_))
					throw std::runtime_error("Permanent error - sorry it didn't work out");
				else
				{ /* non-permanent error */ }
				reason |= should_retry__;
				if (BIO_should_read(bio_))
					reason |= should_read__;
				else
				{ /* no reading requested */ }
				if (BIO_should_write(bio_))
					reason |= should_write__;
				else
				{ /* no write requested */ }
			}
			else
			{ /* all is well */ }
		}
		else
		{ /* nothing to do */ }

		return std::make_pair(written, reason);
	}

	std::pair< std::size_t, int > Connection::read(std::vector< char > & buffer)
	{
		std::size_t bytes_read_into_buffer(0);
		int reason(no_error__);
		bool continue_until_retry(false);
		std::vector< char >::size_type start_offset(0);
		if (buffer.empty())
		{
			continue_until_retry = true;
			buffer.resize(default_read_block_size__);
		}
		else
		{ /* no special handling here */ }
read_entry_point:
		const std::size_t bytes_requested(buffer.size() - start_offset);
		assert(bytes_requested < INT_MAX);
		int bytes_read(::BIO_read(bio_, &(buffer[start_offset]), bytes_requested));
		if (bytes_read < static_cast< int >(bytes_requested))
		{
			bytes_read_into_buffer += bytes_read;
			if (!BIO_should_retry(bio_) && bytes_read <= 0)
				throw std::runtime_error("Permanent error - sorry it didn't work out");
			else
			{
				reason |= should_retry__;
				if (BIO_should_read(bio_))
					reason |= should_read__;
				else
				{ /* no reading requested */ }
				if (BIO_should_write(bio_))
					reason |= should_write__;
				else
				{ /* no write requested */ }
			}
		}
		else if (continue_until_retry)
		{
			bytes_read_into_buffer = buffer.size();
			start_offset = buffer.size();
			buffer.resize(buffer.size() + default_read_block_size__);
			goto read_entry_point;
		}
		else
		{
			bytes_read_into_buffer = buffer.size();
		}

		return std::make_pair(bytes_read_into_buffer, reason);
	}

	bool Connection::usesSSL() const
	{
		SSL * ssl(0);
		BIO_get_ssl(bio_, &ssl);
		return ssl != 0;
	}

	void Connection::setNewDataHandler(Handlers::NewDataHandler & handler)
	{
		data_handler_ = &handler;
		Private::ConnectionHandler::getInstance().attach(BIO_get_fd(bio_, 0), boost::bind(&Connection::onDataReady_, this));
	}

	void Connection::clearNewDataHandler()
	{
		Private::ConnectionHandler::getInstance().detach(BIO_get_fd(bio_, 0));
		data_handler_ = 0;
	}

	Details::Address Connection::getPeerAddress() const
	{
		if (!bio_)
			throw std::logic_error("No connection");
		else
		{ /* carry on */ }
		int socket_fd(::BIO_get_fd(bio_, 0));
		::sockaddr_in peer_addr;
		int peer_addr_size(sizeof(peer_addr));
		::getpeername(socket_fd, (::sockaddr*)&peer_addr, &peer_addr_size);
		return Details::Address(peer_addr.sin_addr.s_addr);
	}

	Connection::Connection(::BIO * bio)
		: bio_(bio)
	{ /* no-op */ }

	void Connection::onDataReady_()
	{
		if (data_handler_)
			(*data_handler_)();
		else
		{ /* no-op */ }
	}
}

