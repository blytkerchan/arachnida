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
				throw std::runtime_error("failed to bind listening socket");	// HERE be more eloquent
			}
			else
			{ /* all is well */ }
			if (::listen(fds_[0], 1) != 0)
			{
				throw std::runtime_error("failed to listen on listening socket");	// HERE be more eloquent
			}
			else
			{ /* all is well */ }
			// get the address we're listening on
			sockaddr_in connection_address;
			SOCKLEN_T addr_size(sizeof(sockaddr_in));
			if (::getsockname(fds_[0], (sockaddr*)&connection_address, &addr_size) != 0 || addr_size != sizeof(sockaddr_in))
			{
				throw std::runtime_error("Failed to obtain address to connect to"); // HERE be more eloquent
			}
			else
			{ /* all is well */ }
			if (::connect(fds_[1], (const sockaddr *)&connection_address, sizeof(sockaddr_in)) != 0)
			{
				throw std::runtime_error("Failed to connect to listening socket"); // HERE be more eloquent
			}
			else
			{ /* all is well */ }
			int reader(::accept(fds_[0], 0, 0));
			if (reader == -1)
				throw std::runtime_error("Failed to accept connection"); // HERE too...
			else
			{ /* all is well */ }
			closeSocket(fds_[0]);
			fds_[0] = reader;
			/************************************************************************/
			int optval(0);
			if (::setsockopt(fds_[0], SOL_SOCKET, SO_RCVBUF, (const char*)&optval, sizeof(int)) == -1) throw std::runtime_error("Failed to set socket option");
			if (::setsockopt(fds_[0], SOL_SOCKET, SO_SNDBUF, (const char*)&optval, sizeof(int)) == -1) throw std::runtime_error("Failed to set socket option");
			if (::setsockopt(fds_[1], SOL_SOCKET, SO_RCVBUF, (const char*)&optval, sizeof(int)) == -1) throw std::runtime_error("Failed to set socket option");
			if (::setsockopt(fds_[1], SOL_SOCKET, SO_SNDBUF, (const char*)&optval, sizeof(int)) == -1) throw std::runtime_error("Failed to set socket option");
			optval = 1;
			if (::setsockopt(fds_[0], SOL_SOCKET, SO_KEEPALIVE, (const char*)&optval, sizeof(int)) == -1) throw std::runtime_error("Failed to set socket option");
			if (::setsockopt(fds_[1], SOL_SOCKET, SO_KEEPALIVE, (const char*)&optval, sizeof(int)) == -1) throw std::runtime_error("Failed to set socket option");
#ifdef ON_WINDOZE
			optval = 1;
			if (::setsockopt(fds_[0], IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(int)) == -1) throw std::runtime_error("Failed to set socket option");
			if (::setsockopt(fds_[1], IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(int)) == -1) throw std::runtime_error("Failed to set socket option");
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
				throw std::runtime_error("Reception from pipe failed"); // HERE be more eloquent
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
#ifdef ON_WINDOZE
					int rc(WSAGetLastError());
					switch (rc)
					{
					case WSANOTINITIALISED : throw std::logic_error("A successful WSAStartup call must occur before using this function."); break;
					case WSAENETDOWN : throw std::runtime_error("The network subsystem has failed."); break;
					case WSAEACCES : throw std::logic_error("The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST socket option to enable use of the broadcast address."); break;
					case WSAEINTR : throw std::runtime_error("A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall."); break;
					case WSAEINPROGRESS : throw std::runtime_error("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function."); break;
					case WSAEFAULT : throw std::runtime_error("The buf parameter is not completely contained in a valid part of the user address space."); break;
					case WSAENETRESET : throw std::runtime_error("The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress."); break;
					case WSAENOBUFS : throw std::runtime_error("No buffer space is available."); break;
					case WSAENOTCONN : throw std::runtime_error("The socket is not connected."); break;
					case WSAENOTSOCK : throw std::logic_error("The descriptor is not a socket."); break;
					case WSAEOPNOTSUPP : throw std::logic_error("MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations."); break;
					case WSAESHUTDOWN : throw std::logic_error("The socket has been shut down; it is not possible to send on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH."); break;
					case WSAEWOULDBLOCK : throw std::runtime_error("The socket is marked as nonblocking and the requested operation would block."); break;
					case WSAEMSGSIZE : throw std::runtime_error("The socket is message oriented, and the message is larger than the maximum supported by the underlying transport."); break;
					case WSAEHOSTUNREACH : throw std::runtime_error("The remote host cannot be reached from this host at this time. "); break;
					case WSAEINVAL : throw std::logic_error("The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled."); break;
					case WSAECONNABORTED : throw std::runtime_error("The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable."); break;
					case WSAECONNRESET : throw std::runtime_error("The virtual circuit was reset by the remote side executing a hard or abortive close. For UPD sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a \"Port Unreachable\" ICMP packet. The application should close the socket as it is no longer usable."); break;
					case WSAETIMEDOUT : throw std::runtime_error("The connection has been dropped, because of a network failure or because the system on the other end went down without notice."); break;
					default : throw std::logic_error("An unknown error occurred"); break;
					}
#else
					switch (errno)
					{
					case EAGAIN :
//#if EGAIN != EWOULDBLOCK
//					case EWOULDBLOCK :
//#endif
						throw std::runtime_error("The socket's file descriptor is marked O_NONBLOCK and the requested operation would block.");
					case EBADF :
						throw std::logic_error("The socket argument is not a valid file descriptor.");
					case ECONNRESET :
						throw std::runtime_error("A connection was forcibly closed by a peer.");
					case EDESTADDRREQ :
						throw std::logic_error("The socket is not connection-mode and no peer address is set.");
					case EINTR :
						throw std::runtime_error("A signal interrupted send() before any data was transmitted.");
					case EMSGSIZE :
						throw std::runtime_error("The message is too large to be sent all at once, as the socket requires.");
					case ENOTCONN :
						throw std::logic_error("The socket is not connected or otherwise has not had the peer pre-specified.");
					case ENOTSOCK :
						throw std::logic_error("The socket argument does not refer to a socket.");
					case EOPNOTSUPP :
						throw std::logic_error("The socket argument is associated with a socket that does not support one or more of the values set in flags.");
					case EPIPE :
						throw std::runtime_error("The socket is shut down for writing, or the socket is connection-mode and is no longer connected.");
					case EACCES :
						throw std::runtime_error("The calling process does not have the appropriate privileges.");
					case EIO :
						throw std::runtime_error("An I/O error occurred while reading from or writing to the file system.");
					case ENETDOWN :
						throw std::runtime_error("The local network interface used to reach the destination is down.");
					case ENETUNREACH :
						throw std::runtime_error("No route to the network is present.");
					case ENOBUFS :
						throw std::runtime_error("Insufficient resources were available in the system to perform the operation.");
					default :
						throw std::logic_error("An unknown error occurred"); break;
					}
#endif
				}
				else
					count -= bytes_sent;
			} while (count);
		}
	}
}
