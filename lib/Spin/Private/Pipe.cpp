#include "Pipe.h"
#include <stdexcept>
#include <loki/ScopeGuard.h>
#if defined(_WIN32) && ! defined(__CYGWIN__)
#include <winsock2.h>
#define closeSocket ::closesocket
#define SOCKLEN_T int
#define SSIZE_T int
#define ON_WINDOZE
#else
#include <cerrno>
extern "C" {
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
}
#define closeSocket ::close
#define SOCKLEN_T socklen_t
#define SSIZE_T ssize_t
#endif
#include "../Exceptions/Connection.h"
#include "../Exceptions/Socket.h"

namespace Spin
{
	namespace Private
	{
		Pipe::Pipe()
		{
			// C++ doesn't allow for array initializations...
			fds_[0] = -1;
			fds_[1] = -1;

			// create the sockets
			fds_[0] = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (fds_[0] == -1)
				throw std::bad_alloc();
			else
			{ /* all is well */ }
			Loki::ScopeGuard fd0_guard = Loki::MakeGuard(closeSocket, fds_[0]);
			fds_[1] = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (fds_[1] == -1)
				throw std::bad_alloc();
			else
			{ /* all is well */ }
			Loki::ScopeGuard fd1_guard = Loki::MakeGuard(closeSocket, fds_[1]);
			sockaddr_in listening_address;
			listening_address.sin_family = AF_INET;
			listening_address.sin_addr.s_addr = ::inet_addr("127.0.0.1");
			listening_address.sin_port = 0; // any ol' port
			if (::bind(fds_[0], (const struct sockaddr *)&listening_address, sizeof(sockaddr_in)) != 0)
			{
				throw Exceptions::Connection::BindFailure(WSAGetLastError());
			}
			else
			{ /* all is well */ }
			if (::listen(fds_[0], 1) != 0)
			{
				throw Exceptions::Connection::ListenFailure(WSAGetLastError());
			}
			else
			{ /* all is well */ }
			// get the address we're listening on
			sockaddr_in connection_address;
			SOCKLEN_T addr_size(sizeof(sockaddr_in));
			if (::getsockname(fds_[0], (sockaddr*)&connection_address, &addr_size) != 0 || addr_size != sizeof(sockaddr_in))
			{
				throw Exceptions::SocketError("Failed to obtain address to connect to", "getsockname", WSAGetLastError());
			}
			else
			{ /* all is well */ }
			if (::connect(fds_[1], (const sockaddr *)&connection_address, sizeof(sockaddr_in)) != 0)
			{
				throw Exceptions::SocketError("Failed to connect to listening socket", "connect", WSAGetLastError());
			}
			else
			{ /* all is well */ }
			int reader(::accept(fds_[0], 0, 0));
			if (reader == -1)
				throw Exceptions::SocketError("Failed to accept connection", "accept", WSAGetLastError());
			else
			{ /* all is well */ }
			closeSocket(fds_[0]);
			fds_[0] = reader;
			/************************************************************************/
			int optval(0);
			if (::setsockopt(fds_[0], SOL_SOCKET, SO_RCVBUF, (const char*)&optval, sizeof(int)) == -1) throw Exceptions::SocketError("Failed to set socket option", "setsockopt", WSAGetLastError());
			if (::setsockopt(fds_[0], SOL_SOCKET, SO_SNDBUF, (const char*)&optval, sizeof(int)) == -1) throw Exceptions::SocketError("Failed to set socket option", "setsockopt", WSAGetLastError());
			if (::setsockopt(fds_[1], SOL_SOCKET, SO_RCVBUF, (const char*)&optval, sizeof(int)) == -1) throw Exceptions::SocketError("Failed to set socket option", "setsockopt", WSAGetLastError());
			if (::setsockopt(fds_[1], SOL_SOCKET, SO_SNDBUF, (const char*)&optval, sizeof(int)) == -1) throw Exceptions::SocketError("Failed to set socket option", "setsockopt", WSAGetLastError());
			optval = 1;
			if (::setsockopt(fds_[0], SOL_SOCKET, SO_KEEPALIVE, (const char*)&optval, sizeof(int)) == -1) throw Exceptions::SocketError("Failed to set socket option", "setsockopt", WSAGetLastError());
			if (::setsockopt(fds_[1], SOL_SOCKET, SO_KEEPALIVE, (const char*)&optval, sizeof(int)) == -1) throw Exceptions::SocketError("Failed to set socket option", "setsockopt", WSAGetLastError());
#ifdef ON_WINDOZE
			optval = 1;
			if (::setsockopt(fds_[0], IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(int)) == -1) throw Exceptions::SocketError("Failed to set socket option", "setsockopt", WSAGetLastError());
			if (::setsockopt(fds_[1], IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(int)) == -1) throw Exceptions::SocketError("Failed to set socket option", "setsockopt", WSAGetLastError());
#endif
			fd0_guard.Dismiss();
			fd1_guard.Dismiss();
		}

		Pipe::~Pipe()
		{
			closeSocket(fds_[1]);
			closeSocket(fds_[0]);
		}

		std::size_t Pipe::read(void * buffer, std::size_t count)
		{
			int bytes_received(::recv(fds_[0], (char*)buffer, count, 0));
			if (bytes_received == -1)
				throw Exceptions::SocketError("Reception from pipe failed", "recv", WSAGetLastError());
			else
			{ /* all is well */ }
			return bytes_received;
		}

		void Pipe::write(const void * buffer, std::size_t count)
		{
			SSIZE_T bytes_sent(0);
			do
			{
				bytes_sent = ::send(fds_[1], (const char *)buffer, count, 0);
				if (bytes_sent == -1)
				{
					throw Exceptions::SocketError("Send failed", "send", WSAGetLastError());
//#else
//					switch (errno)
//					{
//					case EAGAIN :
////#if EGAIN != EWOULDBLOCK
////					case EWOULDBLOCK :
////#endif
//						throw std::runtime_error("The socket's file descriptor is marked O_NONBLOCK and the requested operation would block.");
//					case EBADF :
//						throw std::logic_error("The socket argument is not a valid file descriptor.");
//					case ECONNRESET :
//						throw std::runtime_error("A connection was forcibly closed by a peer.");
//					case EDESTADDRREQ :
//						throw std::logic_error("The socket is not connection-mode and no peer address is set.");
//					case EINTR :
//						throw std::runtime_error("A signal interrupted send() before any data was transmitted.");
//					case EMSGSIZE :
//						throw std::runtime_error("The message is too large to be sent all at once, as the socket requires.");
//					case ENOTCONN :
//						throw std::logic_error("The socket is not connected or otherwise has not had the peer pre-specified.");
//					case ENOTSOCK :
//						throw std::logic_error("The socket argument does not refer to a socket.");
//					case EOPNOTSUPP :
//						throw std::logic_error("The socket argument is associated with a socket that does not support one or more of the values set in flags.");
//					case EPIPE :
//						throw std::runtime_error("The socket is shut down for writing, or the socket is connection-mode and is no longer connected.");
//					case EACCES :
//						throw std::runtime_error("The calling process does not have the appropriate privileges.");
//					case EIO :
//						throw std::runtime_error("An I/O error occurred while reading from or writing to the file system.");
//					case ENETDOWN :
//						throw std::runtime_error("The local network interface used to reach the destination is down.");
//					case ENETUNREACH :
//						throw std::runtime_error("No route to the network is present.");
//					case ENOBUFS :
//						throw std::runtime_error("Insufficient resources were available in the system to perform the operation.");
//					default :
//						throw std::logic_error("An unknown error occurred"); break;
//					}
//#endif
				}
				else
					count -= bytes_sent;
			} while (count);
		}
	}
}
