#include "Pipe.h"
#include <stdexcept>
#include <loki/ScopeGuard.h>
#if defined(_WIN32) && ! defined(__CYGWIN__)
#include <winsock2.h>
#define closeSocket ::closesocket
#else
#error "Not yet implemented"
#define closeSocket ::close
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
			fds_[0] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (fds_[0] == -1)
				throw std::bad_alloc();
			else
			{ /* all is well */ }
			Loki::ScopeGuard fd0_guard = Loki::MakeGuard(closeSocket, fds_[0]);
			fds_[1] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
			int addr_size(sizeof(sockaddr_in));
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
			::setsockopt(fds_[0], SOL_SOCKET, SO_RCVBUF, (const char*)&optval, sizeof(int));
			::setsockopt(fds_[0], SOL_SOCKET, SO_SNDBUF, (const char*)&optval, sizeof(int));
			::setsockopt(fds_[1], SOL_SOCKET, SO_RCVBUF, (const char*)&optval, sizeof(int));
			::setsockopt(fds_[1], SOL_SOCKET, SO_SNDBUF, (const char*)&optval, sizeof(int));
			optval = 1;
			::setsockopt(fds_[0], SOL_SOCKET, SO_KEEPALIVE, (const char*)&optval, sizeof(int));
			::setsockopt(fds_[1], SOL_SOCKET, SO_KEEPALIVE, (const char*)&optval, sizeof(int));
			optval = 1;
			::setsockopt(fds_[0], IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(int));
			::setsockopt(fds_[1], IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(int));
		}

		Pipe::~Pipe()
		{
			closeSocket(fds_[1]);
			closeSocket(fds_[0]);
		}

		void Pipe::read(void * buffer, std::size_t count){}
		void Pipe::write(const void * buffer, std::size_t count){}

	}
}
