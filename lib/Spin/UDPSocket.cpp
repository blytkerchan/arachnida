#include "UDPSocket.h"
#include <Agelena/Logger.h>
#include <boost/bind.hpp>
#include <boost/version.hpp>
#include "Exceptions/Socket.h"
#include "Exceptions/Connection/UnusableUDPSocket.h"
extern "C" {
#if defined(_WIN32) && ! defined(__CYGWIN__)
#	include <WinSock2.h>
#	define ssize_t int
#	define getLastError__ WSAGetLastError
#	define socklen_t int
#else
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <unistd.h>
#	define getLastError__() errno
#	define closesocket ::close
#endif
}
#include <cerrno>
#include "Handlers/UDPDataHandler.h"
#include "Private/ConnectionHandler.h"

#define SOCKET_CALL(statement, check_on_error, message, function)					\
	if ((statement) check_on_error)													\
		throw Exceptions::SocketError(message, function, getLastError__());			\
	else																			\
	{ /* all is well */ }

#ifdef _MSC_VER
#pragma warning(disable: 4389) // == signed/unsigned mismatch - part of FD_SET
#endif

#define AGELENA_COMPONENT_ID	0x5350493eUL
#define AGELENA_SUBCOMPONENT_ID 0x55445053UL

namespace Spin
{
	UDPSocket::UDPSocket(const Details::Address & address, unsigned short port)
		: fd_(-1),
		  data_handler_(0)
	{
		SOCKET_CALL(fd_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP), < 0, "socket allocation failed", "UDPSocket constructor");
		if (port != 0)
		{
			int on(1);
			SOCKET_CALL(setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on)), < 0, "setsockopt failed", "UDPSocket constructor");
			sockaddr_in in_address;
			memset(&in_address, 0, sizeof(in_address));
			in_address.sin_addr.s_addr = address.u_.u32_;
			in_address.sin_family = AF_INET;
			in_address.sin_port = htons(port);
			SOCKET_CALL(bind(fd_, (const sockaddr *)(&in_address), sizeof(sockaddr_in)), < 0, "bind failed", "UDPSocket constructor");
		}
		else
		{ /* no need to bind */ }
	}

	UDPSocket::~UDPSocket()
	{
		clearDataHandler();
		closesocket(fd_);
	}

	std::size_t UDPSocket::send(const Details::Address & to, unsigned short port, const std::vector< char > & data)
	{
		sockaddr_in remote_address;
		memset(&remote_address, 0, sizeof(remote_address));
		remote_address.sin_addr.s_addr = to.u_.u32_;
		remote_address.sin_family = AF_INET;
		remote_address.sin_port = htons(port);

		const char * curr(&data[0]);
		const char * end(curr + data.size());
		if (fd_ == -1)
			throw Exceptions::Connection::UnusableUDPSocket();
		else
		{ /* all is well */ }
		ssize_t sent(::sendto(fd_, curr, std::distance(curr, end), 0, (const sockaddr *)(&remote_address), sizeof(remote_address)));
		if (sent < 0)
		{
			throw Exceptions::SocketError("Send failed", "UDPSocket::send", getLastError__());
		}
		else
		{ /* all is well */ }

		return sent;
	}

	boost::tuple< Details::Address, unsigned short, std::size_t > UDPSocket::recv(std::vector< char > & buffer, unsigned long timeout/* = ~0*/)
	{
		boost::recursive_mutex::scoped_lock sentinel(peek_buffer_lock_);
		if (fd_ == -1)
			throw Exceptions::Connection::UnusableUDPSocket();
		else
		{ /* all is well */ }

		if (boost::tuples::get<3>(peek_buffer_).empty())
		{
			sentinel.unlock();
			bool we_own_the_buffer_size(false);
			if (buffer.size() == 0)
			{
				buffer.resize(4096, 0);
				we_own_the_buffer_size = true;
			}
			else
			{ /* keep the buffer as is */ }
			sockaddr_in remote_address;
			memset(&remote_address, 0, sizeof(remote_address));
			socklen_t remote_address_size(sizeof(remote_address));
			bool timed_out(false);
			if (timeout != ~0)
			{
				int highest_fd = fd_ + 1;
				fd_set read_set;
				fd_set write_set;
				fd_set exc_set;
				FD_ZERO(&read_set);
				FD_ZERO(&write_set);
				FD_ZERO(&exc_set);
				FD_SET(fd_, &read_set);
				timeval time_out;
				time_out.tv_sec = 0;
				time_out.tv_usec = timeout;

				switch (select(highest_fd, &read_set, &write_set, &exc_set, &time_out))
				{
				case 0 :
					timed_out = true;
					break;
				case 1 :
					timed_out = false;
					break;
				default :
					throw Exceptions::SocketError("Select call for recv failed", "UDPSocket::recv", getLastError__());
				}
			}
			else
			{ /* no time-out period */ }
			ssize_t received(timed_out ? 0 : recvfrom(fd_, &buffer[0], buffer.size(), 0, (sockaddr *)(&remote_address), &remote_address_size));
			if (received < 0)
			{
				throw Exceptions::SocketError("Recv failed", "UDPSocket::recv", getLastError__());
			}
			else
			{ /* all is well */ }
			assert(remote_address_size == sizeof(remote_address));
			if (we_own_the_buffer_size)
				buffer.resize(received);
			else
			{ /* we don't own the buffer size, so let the called take care of it */ }
			return boost::make_tuple(Details::Address(remote_address.sin_addr.s_addr), ntohs(remote_address.sin_port), received);
		}
		else
		{
			buffer = boost::tuples::get<3>(peek_buffer_);
			boost::tuples::get<3>(peek_buffer_).clear();
			return boost::make_tuple(boost::tuples::get<0>(peek_buffer_), boost::tuples::get<1>(peek_buffer_), boost::tuples::get<2>(peek_buffer_));
		}
	}

	boost::tuple< Details::Address, unsigned short, std::size_t > UDPSocket::peek(std::vector< char > & buffer, unsigned long timeout/* = ~0*/)
	{
		boost::recursive_mutex::scoped_lock sentinel(peek_buffer_lock_);
		if (fd_ == -1)
			throw Exceptions::Connection::UnusableUDPSocket();
		else
		{ /* all is well */ }
		if (boost::tuples::get<3>(peek_buffer_).empty())
		{
			boost::tie(boost::tuples::get<0>(peek_buffer_), boost::tuples::get<1>(peek_buffer_), boost::tuples::get<2>(peek_buffer_)) = recv(boost::tuples::get<3>(peek_buffer_), timeout);
		}
		else
		{ /* already have a peeked buffer */ }

		buffer = boost::tuples::get<3>(peek_buffer_);
		return boost::make_tuple(boost::tuples::get<0>(peek_buffer_), boost::tuples::get<1>(peek_buffer_), boost::tuples::get<2>(peek_buffer_));
	}

	void UDPSocket::clearPeekBuffer()
	{
		boost::recursive_mutex::scoped_lock sentinel(peek_buffer_lock_);
		boost::tuples::get<3>(peek_buffer_).clear();
	}

	bool UDPSocket::poll() const
	{
		boost::recursive_mutex::scoped_lock sentinel(fd_lock_);
		if (fd_ == -1)
			throw Exceptions::Connection::UnusableUDPSocket();
		else
		{ /* all is well */ }

		bool retval(false);

		int highest_fd = fd_ + 1;
		fd_set read_set;
		fd_set write_set;
		fd_set exc_set;
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		FD_ZERO(&exc_set);
		FD_SET(fd_, &read_set);
		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 1;

		switch (select(highest_fd, &read_set, &write_set, &exc_set, &timeout))
		{
		case 0 :
			retval = false;
			break;
		case 1 :
			retval = true;
			break;
		default :
			throw Exceptions::SocketError("Select call for poll failed", "UDPSocket::poll", getLastError__());
		}

		return retval;
	}

	void UDPSocket::close()
	{
		if (fd_ != -1)
		{
			closesocket(fd_);
			fd_ = -1;
		}
		else
		{ /* already closed */ }
	}

	void UDPSocket::setDataHandler(Handlers::UDPDataHandler & handler, OnErrorCallback on_error_callback/* = OnErrorCallback()*/)
	{
		boost::recursive_mutex::scoped_lock sentinel(fd_lock_);
		if (fd_ == -1)
			throw Exceptions::Connection::UnusableUDPSocket();
		else
		{ /* all is well */ }
		data_handler_ = &handler;
		Private::ConnectionHandler::getInstance().attach(fd_, boost::bind(&UDPSocket::onDataReady_, this), on_error_callback);
	}

	void UDPSocket::clearDataHandler()
	{
		boost::recursive_mutex::scoped_lock sentinel(fd_lock_);
		if (data_handler_ && fd_ != -1)
		{
			AGELENA_DEBUG_1("Detaching FD %1% from the connection handler", fd_);
			Private::ConnectionHandler::getInstance().detach(fd_);
			data_handler_ = 0;
		}
		else
		{ /* nothing to clear */ }
	}

	void UDPSocket::onDataReady_()
	{
		if (data_handler_)
		{
			/* If the internal weak pointer in enable_shared_from_this is 
			 * NULL, we're not stored in a shared_ptr, which means we can't 
			 * pass one to our users. This is possible, for example, if the 
			 * client app uses RAII to handle the life-cycle of the UDP 
			 * socket, rather than using a shared_ptr. */
			/* NOTE: this is only known to work in version 1.33.1 of Boost,
			 *       so using UDPSocket with RAII is only supported in that
			 *       particular version of Boost. The reason for this being 
			 *       here is to accomodate one particular user of Arachnida.
			 *       Use of UDPSocet in this fashion is deprecated. */
#if BOOST_VERSION == 103301
			if (_internal_weak_this.lock())
				(*data_handler_)(shared_from_this());
			else
				(*data_handler_)(this);
#else
			/* NOTE: if your application crashed here or passes you an empty
			 *       shared pointer, it means you are not using version 1.33.1
			 *       of boost and you are using the UDPSocket as a free or 
			 *       stack-based object, rather than through a shared_ptr. 
			 *       If that is the case, you have three avenues before you:
			 *			1.	check whether the version of Boost you are using
			 *				has an enable_shared_from_this with an accessible 
			 *				_internal_weak member and, if so, add it to the 
			 *				exceptions above (and contribute the change back 
			 *				to support@vlinder.ca, please);
			 *			2.	fix your code to use UDPSocket from a shared_ptr
			 *			3.	contact support@vlinder.ca and tell us about your
			 *				problem - we'll be happy to help you fix it. As you 
			 *				can see above, we're happy to adjust our code for
			 *				customers. */
			(*data_handler_)(shared_from_this());
#endif
		}
		else
		{ /* no-op */ }
	}
}

