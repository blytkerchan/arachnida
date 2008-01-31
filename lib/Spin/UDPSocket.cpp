#include "UDPSocket.h"
#include <boost/bind.hpp>
#include "Exceptions/Socket.h"
extern "C" {
#include <WinSock2.h>
}
#define ssize_t int

#include "Handlers/UDPDataHandler.h"
#include "Private/ConnectionHandler.h"

#define SOCKET_CALL(statement, check_on_error, message, function)					\
	if ((statement) check_on_error)													\
		throw Exceptions::SocketError(message, function, WSAGetLastError());		\
	else																			\
	{ /* all is well */ }

namespace Spin
{
	UDPSocket::UDPSocket(const Details::Address & address, unsigned short port)
		: status_(good__),
		  fd_(-1),
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
			in_address.sin_port = port;
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
		checkStatus("UDPSocket::send");
		sockaddr_in remote_address;
		memset(&remote_address, 0, sizeof(remote_address));
		remote_address.sin_addr.s_addr = to.u_.u32_;
		remote_address.sin_family = AF_INET;
		remote_address.sin_port = port;

		const char * curr(&data[0]);
		const char * end(curr + data.size());
		ssize_t sent(::sendto(fd_, curr, std::distance(curr, end), 0, (const sockaddr *)(&remote_address), sizeof(remote_address)));
		if (sent < 0)
		{
			status_ |= error__;
			throw Exceptions::SocketError("Send failed", "UDPSocket::send", WSAGetLastError());
		}
		else
		{ /* all is well */ }

		return sent;
	}

	boost::tuple< Details::Address, unsigned short, std::size_t > UDPSocket::recv(std::vector< char > & buffer, unsigned long timeout/* = ~0*/)
	{
		checkStatus("UDPSocket::recv");
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
		int remote_address_size(sizeof(remote_address));
		ssize_t received(recvfrom(fd_, &buffer[0], buffer.size(), 0, (sockaddr *)(&remote_address), &remote_address_size));
		if (received < 0)
		{
			status_ |= error__;
			throw Exceptions::SocketError("Recv failed", "UDPSocket::recv", WSAGetLastError());
		}
		else
		{ /* all is well */ }
		assert(remote_address_size == sizeof(remote_address));
		if (we_own_the_buffer_size)
			buffer.resize(received);
		else
		{ /* we don't own the buffer size, so let the called take care of it */ }
		return boost::make_tuple(Details::Address(remote_address.sin_addr.s_addr), remote_address.sin_port, received);
	}

	bool UDPSocket::poll() const
	{
		checkStatus("UDPSocket::poll");
		bool retval(false);

		int highest_fd = fd_;
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
			status_ |= error__;
			throw Exceptions::SocketError("Select call for poll failed", "UDPSocket::poll", WSAGetLastError());
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
		status_ |= done__;
	}

	void UDPSocket::setDataHandler(Handlers::UDPDataHandler & handler, OnErrorCallback on_error_callback/* = OnErrorCallback()*/)
	{
		boost::recursive_mutex::scoped_lock sentinel(fd_lock_);
		data_handler_ = &handler;
		Private::ConnectionHandler::getInstance().attach(fd_, boost::bind(&UDPSocket::onDataReady_, this), on_error_callback);
	}

	void UDPSocket::clearDataHandler()
	{
		boost::recursive_mutex::scoped_lock sentinel(fd_lock_);
		if (data_handler_)
		{
			Private::ConnectionHandler::getInstance().detach(fd_);
			data_handler_ = 0;
		}
		else
		{ /* nothing to clear */ }
	}

	void UDPSocket::checkStatus(const char * for_whom) const
	{
		if (status_ != good__)
		{
			status_ |= error__;
			throw Exceptions::SocketError("Socket not usable", for_whom, -1);
		}
		else
		{ /* all is well */ }
	}

	void UDPSocket::onDataReady_()
	{
		if (data_handler_ && status_ == good__)
		{
			/* If the internal weak pointer in enable_shared_from_this is 
			 * NULL, we're not stored in a shared_ptr, which means we can't 
			 * pass one to our users. This is possible, for example, if the 
			 * client app uses RAII to handle the life-cycle of the UDP 
			 * socket, rather than using a shared_ptr. */
			if (_internal_weak_this.lock())
				(*data_handler_)(shared_from_this());
			else
				(*data_handler_)(this);
		}
		else
		{ /* no-op */ }
	}
}

