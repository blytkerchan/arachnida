#include "Connection.h"
#include <stdexcept>
#include <cassert>
#include <climits>
#include <Windows.h>
#include <boost/bind.hpp>
#include <Acari/atomicPrimitives.h>
#include <Scorpion/BIO.h>
#include "Private/ConnectionHandler.h"
#include "Handlers/NewDataHandler.h"
#include "Exceptions/Connection.h"

namespace Spin
{
	/*static */unsigned long Connection::next_attribute_index__(0);

	Connection::Connection(const Connection & connection)
		: bio_(connection.bio_),
		  data_handler_(connection.data_handler_),
		  attributes_(connection.attributes_),
		  status_(connection.status_)
	{
		connection.bio_.reset();
		connection.status_ |= done__;
	}

	Connection::~Connection()
	{
		clearNewDataHandler();
	}

	std::pair< std::size_t, int > Connection::write(const std::vector< char > & data)
	{
		assert(data.size() < INT_MAX);
		if (status_ != good__)
		{
			status_ |= error__;
			throw Exceptions::Connection::UnusableConnection();
		}
		else
		{ /* all is well */ }
		int written(0);
		int reason(no_error__);

		if (!data.empty())
		{
			written = bio_->write(&(data[0]), data.size());
			if (written == -2)
				throw Exceptions::Connection::MethodNotImplemented();
			else if (written < static_cast< int >(data.size()))
			{
				if (!bio_->shouldRetry())
				{
					status_ = done__;
					throw Exceptions::Connection::ConnectionClosed();
				}
				else
				{ /* non-permanent error */ }
				reason |= should_retry__;
				if (bio_->shouldRead())
					reason |= should_read__;
				else
				{ /* no reading requested */ }
				if (bio_->shouldWrite())
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
		if (status_ != good__)
		{
			status_ |= error__;
			throw Exceptions::Connection::UnusableConnection();
		}
		else
		{ /* all is well */ }

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
		int bytes_read(bio_->read(&(buffer[start_offset]), bytes_requested));
		if (bytes_read < static_cast< int >(bytes_requested))
		{
			bytes_read_into_buffer += bytes_read;
			if (!bio_->shouldRetry() && bytes_read <= 0)
			{
				status_ = done__;
				throw Exceptions::Connection::ConnectionClosed();
			}
			else
			{
				reason |= should_retry__;
				if (bio_->shouldRead())
					reason |= should_read__;
				else
				{ /* no reading requested */ }
				if (bio_->shouldWrite())
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
		return bio_->usesSSL();
	}

	/*static */unsigned long Connection::allocateAttribute()
	{
		unsigned long retval(Acari::fetchAndIncrement(next_attribute_index__));
		if (retval >= max_attribute_count__)
			retval = 0xFFFFFFFF;
		else
		{ /* attributes not exhausted */ }
		return retval;
	}

	boost::any & Connection::getAttribute(unsigned long index)
	{
		assert(index < max_attribute_count__);
		return attributes_[index];
	}

	void Connection::setNewDataHandler(Handlers::NewDataHandler & handler, OnErrorCallback on_error_callback/* = OnErrorCallback()*/)
	{
		data_handler_ = &handler;
		Private::ConnectionHandler::getInstance().attach(bio_->getFD(), boost::bind(&Connection::onDataReady_, this), on_error_callback);
	}

	void Connection::clearNewDataHandler()
	{
		if (data_handler_)
		{
			Private::ConnectionHandler::getInstance().detach(bio_->getFD());
			data_handler_ = 0;
		}
		else
		{ /* nothing to clear */ }
	}

	Details::Address Connection::getPeerAddress() const
	{
		if (!bio_ || status_ != good__)
			throw std::logic_error("No connection");
		else
		{ /* carry on */ }
		int socket_fd(bio_->getFD());
		::sockaddr_in peer_addr;
		int peer_addr_size(sizeof(peer_addr));
		::getpeername(socket_fd, (::sockaddr*)&peer_addr, &peer_addr_size);
		return Details::Address(peer_addr.sin_addr.s_addr);
	}

	Connection::Connection(Scorpion::BIO * bio)
		: bio_(bio),
		  data_handler_(0),
		  attributes_(max_attribute_count__),
		  status_(good__)
	{ /* no-op */ }

	void Connection::onDataReady_()
	{
		if (data_handler_ && status_ == good__)
			(*data_handler_)(*this);
		else
		{ /* no-op */ }
	}
}

