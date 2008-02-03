#include "Connection.h"
#include <stdexcept>
#include <cassert>
#include <climits>
#if defined(_WIN32) && ! defined(__CYGWIN__)
#include <Windows.h>
#define socklen_t int
#else
extern "C" {
#include <sys/socket.h>
#include <netinet/in.h>
}
#endif
#include <boost/bind.hpp>
#include <Acari/atomicPrimitives.h>
#include <Scorpion/BIO.h>
#include <Agelena/Logger.h>
#include "Private/ConnectionHandler.h"
#include "Handlers/NewDataHandler.h"
#include "Exceptions/Connection.h"

namespace Spin
{
	Connection::~Connection()
	{
		AGELENA_DEBUG_1("Connection(%1%)::~Connection()", this);
		clearNewDataHandler();
	}

	std::pair< std::size_t, int > Connection::write(const std::vector< char > & data)
	{
		AGELENA_DEBUG_1("std::pair< std::size_t, int > Connection(%1%)::write(const std::vector< char > & data)", this);
		boost::recursive_mutex::scoped_lock sentinel(bio_lock_);
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
		AGELENA_DEBUG_1("std::pair< std::size_t, int > Connection(%1%)::read(std::vector< char > & buffer)", this);
		boost::recursive_mutex::scoped_lock sentinel(bio_lock_);
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

	bool Connection::poll() const
	{
		AGELENA_DEBUG_1("bool Connection(%1%)::poll() const", this);
		boost::recursive_mutex::scoped_lock sentinel(bio_lock_);
		if (status_ != good__)
		{
			status_ |= error__;
			throw Exceptions::Connection::UnusableConnection();
		}
		else
		{ /* all is well */ }
		return bio_->poll();
	}

	void Connection::close()
	{
		AGELENA_DEBUG_1("void Connection(%1%)::close()", this);
		boost::recursive_mutex::scoped_lock sentinel(bio_lock_);
		bio_.reset();
		status_ |= done__;
	}

	bool Connection::usesSSL() const
	{
		AGELENA_DEBUG_1("bool Connection(%1%)::usesSSL() const", this);
		boost::recursive_mutex::scoped_lock sentinel(bio_lock_);
		if (status_ != good__)
		{
			status_ |= error__;
			throw Exceptions::Connection::UnusableConnection();
		}
		else
		{ /* all is well */ }
		return bio_->usesSSL();
	}

	void Connection::setNewDataHandler(Handlers::NewDataHandler & handler, OnErrorCallback on_error_callback/* = OnErrorCallback()*/)
	{
		AGELENA_DEBUG_1("void Connection(%1%)::setNewDataHandler(Handlers::NewDataHandler & handler, OnErrorCallback on_error_callback/* = OnErrorCallback()*/)", this);
		boost::recursive_mutex::scoped_lock sentinel(bio_lock_);
		data_handler_ = &handler;
		Private::ConnectionHandler::getInstance().attach(fd_, boost::bind(&Connection::onDataReady_, this), on_error_callback);
	}

	void Connection::clearNewDataHandler()
	{
		AGELENA_DEBUG_1("void Connection(%1%)::clearNewDataHandler()", this);
		boost::recursive_mutex::scoped_lock sentinel(bio_lock_);
		if (data_handler_)
		{
			Private::ConnectionHandler::getInstance().detach(fd_);
			data_handler_ = 0;
		}
		else
		{ /* nothing to clear */ }
	}

	Details::Address Connection::getPeerAddress() const
	{
		AGELENA_DEBUG_1("Details::Address Connection(%1%)::getPeerAddress() const", this);
		boost::recursive_mutex::scoped_lock sentinel(bio_lock_);
		if (!bio_ || status_ != good__)
			throw std::logic_error("No connection");
		else
		{ /* carry on */ }
		int socket_fd(fd_);
		::sockaddr_in peer_addr;
		socklen_t peer_addr_size(sizeof(peer_addr));
		::getpeername(socket_fd, (::sockaddr*)&peer_addr, &peer_addr_size);
		return Details::Address(peer_addr.sin_addr.s_addr);
	}

	Connection::Connection(Scorpion::BIO * bio)
		: bio_(bio),
		  data_handler_(0),
		  status_(good__),
		  fd_(bio->getFD())
	{
		AGELENA_DEBUG_1("Connection(%1%)::Connection(Scorpion::BIO * bio)", this);
	}

	void Connection::onDataReady_()
	{
		AGELENA_DEBUG_1("void Connection(%1%)::onDataReady_()", this);
		if (data_handler_ && status_ == good__)
			(*data_handler_)(shared_from_this());
		else
		{ /* no-op */ }
	}
}

